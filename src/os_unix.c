/******************************************************************
 * FabricDB Library OS operations Implementation for Unix
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: June 22, 2016
 * Modified: June 25, 2016
 * Author: Mark Wardle
 * Description:
 *     Defines low-level, os-specific file operations.
 *     Much inspiration is taken from sqlite3 (why resolve
 *     a solved problem?).
 *
 ******************************************************************/

#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "fabric.h"
#include "os.h"
#include "mem.h"
#include "mutex.h"

#define DEFAULT_FILE_PERMS (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)        // 0644
#define DEFAULT_DIR_PERMS (S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) // 0755

#define PENDING_BYTE 0x40000000
#define RESERVED_BYTE (PENDING_BYTE + 1)
#define SHARED_BYTE (PENDING_BYTE + 2)

#define MIN_FILE_DESCRIPTOR 3

#ifndef stat_t
typedef struct stat stat_t;
#endif

/******************************************************************
 * FILE DESCRIPTOR OPS
 ******************************************************************/
static int fdb_fd_open(const char* filePath, int flags, int perms){
    int fd;
    while(1) {
        fd = open(filePath, flags, perms);
        if(fd < 0 && errno == EINTR){
            continue;
        }
        break;
    }

    return fd;
}

static ssize_t fdb_fd_read(int fd, uint8_t* buffer, off_t begin, size_t count){
    return pread(fd, buffer, count, begin);
}

static ssize_t fdb_fd_write(int fd, uint8_t* buffer, off_t begin, size_t count){
    return pwrite(fd, buffer, count, begin);
}

static int fdb_fd_open_read_write(const char* path){
    return open(path, O_RDWR, DEFAULT_FILE_PERMS);
}

static int fdb_fd_open_read_only(const char* path){
    return open(path, O_RDONLY, DEFAULT_FILE_PERMS);
}

static int fdb_fd_create_read_write(const char* path){
    return open(path, O_RDWR|O_CREAT|O_EXCL, DEFAULT_FILE_PERMS);
}

static int fdb_ioerror_from_errno() {
    int fdberrno = FABRICDB_EIO;
    switch(errno) {
        case EACCES:
            fdberrno = FABRICDB_EACCES;
            break;
        case EEXIST:
            fdberrno = FABRICDB_EEXIST;
            break;
        case EISDIR:
            fdberrno = FABRICDB_EISDIR;
            break;
        case ELOOP:
            fdberrno = FABRICDB_ELOOP;
            break;
        case EMFILE:
            fdberrno = FABRICDB_EMFILE;
            break;
        case ENAMETOOLONG:
            fdberrno = FABRICDB_ENAMETOOLONG;
            break;
        case ENFILE:
            fdberrno = FABRICDB_ENFILE;
            break;
        case ENOENT:
            fdberrno = FABRICDB_ENOENT;
            break;
        case ENOSPC:
            fdberrno = FABRICDB_ENOSPC;
            break;
        case ENOTDIR:
            fdberrno = FABRICDB_ENOTDIR;
            break;
        case EOVERFLOW:
            fdberrno = FABRICDB_EOVERFLOW;
            break;
        case EINVAL:
            fdberrno = FABRICDB_EINVAL;
            break;
        case EFBIG:
            fdberrno = FABRICDB_EFBIG;
            break;
        case EBADF:
            fdberrno = FABRICDB_EBADF;
            break;
        case ENOBUFS:
            fdberrno = FABRICDB_ENOBUFS;
            break;
        case ENOMEM:
            fdberrno = FABRICDB_ENOMEM;
            break;
    }

    return fdberrno;
}

/******************************************************************
 * FILE DESCRIPTOR LOCKING
 ******************************************************************/
int fdb_check_reserved_lock(int fd){
    struct flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_start = RESERVED_BYTE;
    lock.l_len = 1;
    lock.l_type = F_WRLCK;

    if(fcntl(fd, F_GETLK, &lock)) {
        return -1;
    }
    if (lock.l_type != F_UNLCK) {
        return 1;
    }
    return 0;
}

static int set_lock(int fd, off_t start, int type){
    struct flock lock;
    int rc;

    lock.l_whence = SEEK_SET;
    lock.l_start = start;
    lock.l_len = 1;
    lock.l_type = type;

    return fcntl(fd, F_SETLK, &lock);
}

/* shared byte */
static int fdb_unlock_shared_byte(int fd){return set_lock(fd, SHARED_BYTE, F_UNLCK);}
static int fdb_readlock_shared_byte(int fd){return set_lock(fd, SHARED_BYTE, F_RDLCK);}
static int fdb_writelock_shared_byte(int fd){return set_lock(fd, SHARED_BYTE, F_WRLCK);}

/* pending byte */
static int fdb_unlock_pending_byte(int fd){return set_lock(fd, PENDING_BYTE, F_UNLCK);}
static int fdb_readlock_pending_byte(int fd){return set_lock(fd, PENDING_BYTE, F_RDLCK);}
static int fdb_writelock_pending_byte(int fd){return set_lock(fd, PENDING_BYTE, F_WRLCK);}

/* reserved byte */
static int fdb_unlock_reserved_byte(int fd){return set_lock(fd, RESERVED_BYTE, F_UNLCK);}
static int fdb_readlock_reserved_byte(int fd){return set_lock(fd, RESERVED_BYTE, F_RDLCK);}
static int fdb_writelock_reserved_byte(int fd){return set_lock(fd, RESERVED_BYTE, F_WRLCK);}

/******************************************************************
 * UNUSED FILE HANDLE
 ******************************************************************/
typedef struct UnusedFileHandle {
    int fd;
    struct UnusedFileHandle *next;
} UnusedFileHandle;

/******************************************************************
 * FILE ID
 ******************************************************************/
typedef struct FileId {
    dev_t deviceNumber;
    ino_t inodeNumber;
} FileId;

/******************************************************************
 * INODE INFO
 ******************************************************************/
typedef struct InodeInfo {
    FileId fileId;
    int sharedLockCount;
    int lockLevel;
    int refCount;
    int lockCount;
    UnusedFileHandle *unusedFiles;   /* singley-linked list of unused files */
    struct InodeInfo* next;
    struct InodeInfo* prev;
} InodeInfo;

InodeInfo* fdb_inodeinfo_new(FileId fileId) {
    InodeInfo* info = fdbmalloc(sizeof(InodeInfo));
    if (info == NULL) {
        return NULL;
    }

    info->fileId = fileId;
    info->sharedLockCount = 0;
    info->lockLevel = 0;
    info->refCount = 0;
    info->lockCount = 0;
    info->unusedFiles = NULL;
    info->next = NULL;
    info->prev = NULL;

    return info;
}

static InodeInfo* inodeInfoList = NULL;

static int fdb_inodeinfo_fetch(int fd, InodeInfo** iip) {
    /* The inode info mutex must be held before calling this function */

    stat_t st;
    FileId fileId;
    InodeInfo *info = inodeInfoList;

    if (fstat(fd, &st) == -1){
        return FABRICDB_EIO;
    }

    fileId.deviceNumber = st.st_dev;
    fileId.inodeNumber = st.st_ino;

    while(info != NULL) {
        if (
            info->fileId.deviceNumber == fileId.deviceNumber &&
            info->fileId.inodeNumber == fileId.inodeNumber
        ) {
            /* We found it, so return it. */
            *iip = info;
            return FABRICDB_OK;
        }
        info = info->next;
    }

    /* The inode info wasn't found, so we make a new one. */
    info = fdb_inodeinfo_new(fileId);
    if (info == NULL) {
        return FABRICDB_ENOMEM;
    }
    if (inodeInfoList != NULL) {
        inodeInfoList->prev = info;
    }
    info->next = inodeInfoList;
    inodeInfoList = info;

    *iip = info;

    return FABRICDB_OK;
}

void fdb_inodeinfo_add_reference(InodeInfo *info) {
    info->refCount++;
}

static void fdb_inodeinfo_remove_reference(InodeInfo *info) {
    info->refCount--;

    if (info->refCount < 1) {
        if (inodeInfoList == info) {
            inodeInfoList = info->next;
        }
        if (info->next != NULL) {
            info->next->prev = info->prev;
        }
        if (info->prev != NULL) {
            info->prev->next = info->next;
        }

        fdbfree(info);
    }
}

static void fdb_inodeinfo_close_unused_files(InodeInfo *info) {
    UnusedFileHandle* ufh = info->unusedFiles;
    UnusedFileHandle* next;
    while(ufh != NULL) {
        close(ufh->fd);
        next = ufh->next;
        fdbfree(ufh);
        ufh = next;
    }

    info->unusedFiles = NULL;
}

/******************************************************************
 * FILE HANDLE
 ******************************************************************/
typedef struct FileHandle {
    int fd;                /* The file handle */
    char*      filePath;   /* The path used to open the file */
    int        lockLevel;  /* The file lock level this file handle currently has */
    InodeInfo* inodeInfo;  /* Shared among threads */
} FileHandle;

static FileHandle* fdb_filehandle_new(int fd, const char* filePath, InodeInfo* inodeInfo){
    FileHandle* fh = fdbmalloc(sizeof(FileHandle));
    if (fh == NULL) {
        return NULL;
    }

    char* filePathCopy = fdbmalloc(strlen(filePath));
    if (filePathCopy == NULL) {
        fdbfree(fh);
        return NULL;
    }

    strcpy(filePathCopy, filePath);

    fh->fd = fd;
    fh->filePath = filePathCopy;
    fh->inodeInfo = inodeInfo;

    return fh;
}

static FileHandle* fdb_filehandle_destroy(FileHandle* fh) {
    fdbfree(fh->filePath);
    fdbfree(fh);
}

static int fdb_filehandle_open(const char* filePath, int flags, FileHandle **fhp) {

    /* Open the file descriptor */
    InodeInfo* info;
    int rc = FABRICDB_OK;
    int fd = fdb_fd_open(filePath, flags, DEFAULT_FILE_PERMS);
    FileHandle* fh = NULL;
    if (fd < 0) {
        return fdb_ioerror_from_errno();
    }
    else if (fd < MIN_FILE_DESCRIPTOR) {
        /* likely opened one of stdout, stderr, or stdin
           which will surely not work as expected */
        return FABRICDB_EINVALID_FILE;
    }


    /* Secure the inode info mutex */
    fdb_enter_mutex(FDB_INODE_MUTEX);

    /* Determine the inode structure */
    rc = fdb_inodeinfo_fetch(fd, &info);
    if (rc != FABRICDB_OK) {
        goto filehandle_open_done;
    }
    fdb_inodeinfo_add_reference(info);

    /* Create the file handle object */
    fh = fdb_filehandle_new(fd, filePath, NULL);
    if (fh == NULL) {
        rc = FABRICDB_ENOMEM;
        fdb_inodeinfo_remove_reference(info);
        goto filehandle_open_done;
    }

    fh->inodeInfo = info;

    filehandle_open_done:
    fdb_leave_mutex(FDB_INODE_MUTEX);
    *fhp = fh;
    return rc;
}

/******************************************************************
 * PUBLIC FILE OPS API
 ******************************************************************/
int fdb_open_file_rdwr(const char *filePath, FileHandle **fhp) {
    return fdb_filehandle_open(filePath, O_RDWR, fhp);
}

int fdb_open_file_rdonly(const char *filePath, FileHandle **fhp) {
    return fdb_filehandle_open(filePath, O_RDONLY, fhp);
}

int fdb_create_file(const char *filePath, FileHandle **fhp) {
    return fdb_filehandle_open(filePath, O_RDWR|O_CREAT|O_EXCL, fhp);
}

int fdb_close_file(FileHandle *fh) {
    /* It may not be correct to actually close the file immediately
       since it can screw with the file locks held by the process.
       As such, an unclosed file will be tracked by the inode info
       and closed at an appropriate time. */

    int fd = fh->fd;
    UnusedFileHandle *ufh;
    InodeInfo *info = fh->inodeInfo;

    fdb_enter_mutex(FDB_INODE_MUTEX);

    if (info->lockCount < 1) {
        close(fd);
        fdb_filehandle_destroy(fh);
        fdb_inodeinfo_remove_reference(info);
        fdb_leave_mutex(FDB_INODE_MUTEX);
        return FABRICDB_OK;
    }

    ufh = fdbmalloc(sizeof(UnusedFileHandle));
    if (ufh == NULL) {
       fdb_leave_mutex(FDB_INODE_MUTEX);
       return FABRICDB_ENOMEM;
    }

    ufh->fd = fd;
    ufh->next = info->unusedFiles;
    info->unusedFiles = info;
    fdb_filehandle_destroy(fh);
    fdb_inodeinfo_remove_reference(info);

    fdb_leave_mutex(FDB_INODE_MUTEX);

    return FABRICDB_OK;
}

int fdb_truncate_file(FileHandle *fh, off_t size) {
    if (ftruncate(fh->fd, size) != 0) {
        return fdb_ioerror_from_errno();
    }

    return FABRICDB_OK;
}

int fdb_file_size(FileHandle *fh, off_t *out) {
    stat_t st;

    if (fstat(fh->fd, &st) == -1){
        return FABRICDB_EIO;
    }

    /* This assumes the file is not a symbolic link.
       This needs to be fixed before it is forgotten about.
       TODO */


    *out = st.st_size;
    return FABRICDB_OK;
}

int fdb_read(FileHandle *fh, uint8_t *dest, off_t offset, size_t num_bytes) {
    size_t nRead;
    while(1) {
        nRead = pread(fh->fd, dest, num_bytes, offset);
        if (nRead == -1) {
            if (errno == EINTR) {
                continue;
            }
            return fdb_ioerror_from_errno();
        }
        if (nRead < num_bytes) {
            return FABRICDB_ESHORTREAD;
        }
        return FABRICDB_OK;
    }
}

int fdb_write(FileHandle *fh, uint8_t *content, off_t offset, size_t num_bytes) {
    size_t nWritten;
    while(1){
        nWritten = pwrite(fh->fd, content, num_bytes, offset);
        if (nWritten == -1) {
            if (errno == EINTR) {
                continue;
            }
            return fdb_ioerror_from_errno();
        }
        if (nWritten < num_bytes) {
            return FABRICDB_ESHORTWRITE;
        }
        return FABRICDB_OK;
    }
}

int fdb_sync(FileHandle *fh) {
    while(fsync(fh->fd) == -1) {
        if (errno == EINTR) {
            continue;
        }
        return fdb_ioerror_from_errno();
    }

    return FABRICDB_OK;
}

/******************************************************************
 * PUBLIC FILE LOCKING ROUTINES
 ******************************************************************/
int fdb_acquire_shared_lock(FileHandle *fh) {
    int rc = FABRICDB_OK;
    InodeInfo *info;

    assert(fh);

    if (fh->lockLevel >= FDB_SHARED_LOCK) {
        /* A lock that is at least this level has already
           been acquired, so do nothing. */
        return FABRICDB_OK;
    }

    fdb_enter_mutex(FDB_INODE_MUTEX);
    info = fh->inodeInfo;

    if (info->lockLevel >= FDB_PENDING_LOCK) {
        rc = FABRICDB_BUSY;
        goto end_acquire_shared_lock;
    }

    if (info->lockLevel == FDB_SHARED_LOCK || info->lockLevel == FDB_RESERVED_LOCK) {
        assert(info->sharedLockCount > 0);
        fh->lockLevel = FDB_SHARED_LOCK;
        info->sharedLockCount++;
        info->lockCount++;
        goto end_acquire_shared_lock;
    }

    assert(info->sharedLockCount == 0);
    assert(info->lockLevel == FDB_NO_LOCK);

    if (fdb_readlock_pending_byte(fh->fd) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            rc = FABRICDB_BUSY;
        }
        else {
            rc = fdb_ioerror_from_errno();
        }
        goto end_acquire_shared_lock;
    }

    if (fdb_readlock_shared_byte(fh->fd) == -1) {
        rc = fdb_ioerror_from_errno();
    }

    if (fdb_unlock_pending_byte(fh->fd) == -1 && rc == FABRICDB_OK){
        rc = fdb_ioerror_from_errno();
    }

    if (rc == FABRICDB_OK) {
        info->lockLevel = FDB_SHARED_LOCK;
        info->lockCount++;
        info->sharedLockCount = 1;
    }

    end_acquire_shared_lock:
    fdb_leave_mutex(FDB_INODE_MUTEX);
    return rc;
}

int fdb_acquire_reserved_lock(FileHandle *fh) {
    int rc = FABRICDB_OK;
    InodeInfo *info;

    assert(fh);
    assert(fh->lockLevel >= FDB_SHARED_LOCK);

    if (fh->lockLevel >= FDB_RESERVED_LOCK) {
        /* A lock that is at least this level has already
           been acquired, so do nothing. */
        return FABRICDB_OK;
    }

    fdb_enter_mutex(FDB_INODE_MUTEX);
    info = fh->inodeInfo;

    if(info->lockLevel >= FDB_RESERVED_LOCK) {
        rc = FABRICDB_BUSY;
        goto end_acquire_reserved_lock;
    }

    if(fdb_writelock_reserved_byte(fh->fd) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            rc = FABRICDB_BUSY;
        } else {
            rc = fdb_ioerror_from_errno();
        }
    }

    if (rc == FABRICDB_OK) {
        fh->lockLevel = FDB_RESERVED_LOCK;
        info->lockLevel = FDB_RESERVED_LOCK;
    }

    end_acquire_reserved_lock:
    fdb_leave_mutex(FDB_INODE_MUTEX);
    return rc;
}

int fdb_acquire_exclusive_lock(FileHandle *fh) {
    int rc = FABRICDB_OK;
    InodeInfo *info;

    assert(fh);
    assert(fh->lockLevel >= FDB_SHARED_LOCK);

    if (fh->lockLevel == FDB_EXCLUSIVE_LOCK) {
        /* A lock that is at least this level has already
           been acquired, so do nothing. */
        return FABRICDB_OK;
    }

    fdb_enter_mutex(FDB_INODE_MUTEX);
    info = fh->inodeInfo;

    if (info->lockLevel != fh->lockLevel && info->lockLevel >= FDB_RESERVED_LOCK) {
        rc = FABRICDB_BUSY;
        goto end_acquire_exclusive_lock;
    }

    if (fh->lockLevel < FDB_PENDING_LOCK) {
        if (fdb_writelock_pending_byte(fh->fd) == -1) {
            if (errno == EACCES || errno == EAGAIN) {
                rc = FABRICDB_BUSY;
            } else {
                rc = fdb_ioerror_from_errno();
            }
            goto end_acquire_exclusive_lock;
        }
    }

    fh->lockLevel = FDB_PENDING_LOCK;
    info->lockLevel = FDB_PENDING_LOCK;

    if (info->sharedLockCount > 1) {
        rc = FABRICDB_BUSY;
        goto end_acquire_exclusive_lock;
    }

    if (fdb_writelock_shared_byte(fh->fd) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            rc = FABRICDB_BUSY;
        } else {
            rc = fdb_ioerror_from_errno();
        }
    }

    if(rc == FABRICDB_OK) {
        fh->lockLevel = FDB_EXCLUSIVE_LOCK;
        info->lockLevel = FDB_EXCLUSIVE_LOCK;
    }

    end_acquire_exclusive_lock:
    fdb_leave_mutex(FDB_INODE_MUTEX);
    return rc;
}

int fdb_downgrade_lock(FileHandle *fh) {
    int rc = FABRICDB_OK;
    InodeInfo *info;

    assert(fh);

    if(fh->lockLevel <= FDB_SHARED_LOCK) {
        return FABRICDB_OK;
    }

    fdb_enter_mutex(FDB_INODE_MUTEX);
    info = fh->inodeInfo;

    assert(fh->lockLevel == info->lockLevel);

    if(
        fdb_readlock_shared_byte(fh->fd) == -1 ||
        fdb_unlock_pending_byte(fh->fd) == -1 ||
        fdb_unlock_reserved_byte(fh->fd) == -1
    ) {
        rc = fdb_ioerror_from_errno();
        goto end_downgrade_lock;
    }

    fh->lockLevel = FDB_SHARED_LOCK;
    info->lockLevel = FDB_SHARED_LOCK;

    end_downgrade_lock:
    fdb_leave_mutex(FDB_INODE_MUTEX);
    return rc;
}

int fdb_unlock(FileHandle *fh) {
    int rc = FABRICDB_OK;
    InodeInfo *info;

    assert(fh);

    if(fh->lockLevel < FDB_SHARED_LOCK) {
        return FABRICDB_OK;
    }

    if(fh->lockLevel > FDB_NO_LOCK) {
        rc = fdb_downgrade_lock(fh);
        if (rc != FABRICDB_OK) {
            return rc;
        }
    }

    fdb_enter_mutex(FDB_INODE_MUTEX);
    info = fh->inodeInfo;

    assert(info->sharedLockCount != 0);

    info->sharedLockCount--;
    assert(info->sharedLockCount >= 0);
    if (info->sharedLockCount == 0){
        if (fdb_unlock_shared_byte(fh->fd) == -1) {
            rc = fdb_ioerror_from_errno();
        }

        info->lockLevel == FDB_NO_LOCK;
    }

    info->lockCount--;
    assert(info->lockCount >= 0);
    if (info->lockCount == 0) {
        fdb_inodeinfo_close_unused_files(info);
    }

    fh->lockLevel = FDB_NO_LOCK;

    end_unlock:
    fdb_leave_mutex(FDB_INODE_MUTEX);
    return rc;
}
