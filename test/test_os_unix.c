#include "test_common.h"

static const char* TEMPFILENAME = "./tempfile.tmp";
static const char* TEMPFILENAME_2 = "./tempfile2.tmp";
static const char* TEMPFILENAME_3 = "./tempfile3.tmp";
static const char* FAKEFILENAME = "./fakefile.tmp";

void test_create_file() {
    FileHandle *fh;
    int rc;

    remove(TEMPFILENAME);

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);

    rc = fdb_create_file(TEMPFILENAME, &fh);
    fdb_assert("Opening the file was not successful", FABRICDB_OK == rc);

    fdb_assert("Returned null", fh);

    fdb_assert("Did not open file descriptor", fh->fd >= MIN_FILE_DESCRIPTOR);
    fdb_assert("Did not copy file path", TEMPFILENAME != fh->filePath);
    fdb_assert("Did not set file path", fh->filePath);
    fdb_assert("Lock level not zeroed", fh->lockLevel == 0);
    fdb_assert("Inode Info not set", fh->inodeInfo);
    fdb_assert("Did not track mem usage", fabricdb_mem_used() > 0);

    fdb_assert("Closing file failed", !fdb_close_file(fh));

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    rc = fdb_create_file(TEMPFILENAME, &fh);
    fdb_assert("File was overwritten", FABRICDB_OK != rc);
    fdb_assert("Did not return null pointer", fh == NULL);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_open_file_rdwr() {
    FileHandle *fh;
    int rc;

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);
    rc = fdb_open_file_rdwr(TEMPFILENAME, &fh);
    fdb_assert("Opening the file was not successful", FABRICDB_OK == rc);
    fdb_assert("Did not open file descriptor", fh->fd >= MIN_FILE_DESCRIPTOR);
    fdb_assert("Did not copy file path", TEMPFILENAME != fh->filePath);
    fdb_assert("Did not set file path", fh->filePath);
    fdb_assert("Lock level not zeroed", fh->lockLevel == 0);
    fdb_assert("Inode Info not set", fh->inodeInfo);
    fdb_assert("Did not track mem usage", fabricdb_mem_used() > 0);

    fdb_assert("Closing file failed", !fdb_close_file(fh));
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    rc = fdb_open_file_rdwr(FAKEFILENAME, &fh);
    fdb_assert("Didn't fail when it should have", FABRICDB_OK != rc);
    fdb_assert("Did not return null pointer", fh == NULL);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_open_file_rdonly() {
    FileHandle *fh;
    int rc;

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);
    rc = fdb_open_file_rdonly(TEMPFILENAME, &fh);
    fdb_assert("Opening the file was not successful", FABRICDB_OK == rc);
    fdb_assert("Did not open file descriptor", fh->fd >= MIN_FILE_DESCRIPTOR);
    fdb_assert("Did not copy file path", TEMPFILENAME != fh->filePath);
    fdb_assert("Did not set file path", fh->filePath);
    fdb_assert("Lock level not zeroed", fh->lockLevel == 0);
    fdb_assert("Inode Info not set", fh->inodeInfo);
    fdb_assert("Did not track mem usage", fabricdb_mem_used() > 0);

    fdb_assert("Closing file failed", !fdb_close_file(fh));
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    rc = fdb_open_file_rdonly(FAKEFILENAME, &fh);
    fdb_assert("Didn't fail when it should have", FABRICDB_OK != rc);
    fdb_assert("Did not return null pointer", fh == NULL);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_inodeinfo_fetch() {
    FileHandle *fh1;
    FileHandle *fh2;
    FileHandle *fh3;
    FileHandle *fh4;

    remove(TEMPFILENAME_2);
    remove(TEMPFILENAME_3);

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);
    fdb_assert("Inode info list is not null", inodeInfoList == NULL);

    fdb_assert("Could not open file", 0 == fdb_open_file_rdwr(TEMPFILENAME, &fh1));
    fdb_assert("Null file handle", fh1);
    fdb_assert("Inode not set", fh1->inodeInfo);
    fdb_assert("Reference count not set", fh1->inodeInfo->refCount == 1);
    fdb_assert("Inode info list not set", inodeInfoList == fh1->inodeInfo);
    fdb_assert("Inode prev is not null", inodeInfoList->next == NULL);
    fdb_assert("Inode next not set", inodeInfoList->next == NULL);

    fdb_assert("Could not open file", 0 == fdb_open_file_rdwr(TEMPFILENAME, &fh2));

    fdb_assert("Null file handle", fh2);
    fdb_assert("Inode not set", fh2->inodeInfo);
    fdb_assert("Reference count not updated", fh2->inodeInfo->refCount == 2);
    fdb_assert("Inode prev is not null", inodeInfoList->next == NULL);
    fdb_assert("Inode next not set", inodeInfoList->next == NULL);

    fdb_assert("Have same file descriptor", fh1->fd != fh2->fd);
    fdb_assert("Have different inode infos", fh1->inodeInfo == fh2->inodeInfo);
    fdb_assert("Has a non null unused files", fh1->inodeInfo->unusedFiles == NULL);

    fdb_assert("Could not open file", 0 == fdb_create_file(TEMPFILENAME_2, &fh3));
    fdb_assert("Null file handle", fh3);
    fdb_assert("Inode not set", fh3->inodeInfo);
    fdb_assert("Reference count not updated", fh2->inodeInfo->refCount == 2);
    fdb_assert("Reference count not updated", fh3->inodeInfo->refCount == 1);

    fdb_assert("Inode list not updated", inodeInfoList == fh3->inodeInfo);
    fdb_assert("Inode prev is not null", inodeInfoList->prev == NULL);
    fdb_assert("Inode next not set", inodeInfoList->next == fh2->inodeInfo);
    fdb_assert("Inode prev not set", inodeInfoList->next->prev == inodeInfoList);
    fdb_assert("Inode next set", inodeInfoList->next->next == NULL);

    fdb_open_file_rdwr(TEMPFILENAME, &fh4);
    fdb_assert("Inode not found correctly", fh4->inodeInfo == fh1->inodeInfo);
    fdb_assert("Inode refCount not incremented", fh4->inodeInfo->refCount == 3);

    fdb_close_file(fh4);

    fdb_close_file(fh3);
    fdb_assert("Inode list not updated", inodeInfoList == fh1->inodeInfo);
    fdb_assert("Inode prev is not null", inodeInfoList->prev == NULL);
    fdb_assert("Inode next is not null", inodeInfoList->next == NULL);

    fdb_close_file(fh1);
    // no locks yet
    fdb_assert("Has a non null unused files", fh2->inodeInfo->unusedFiles == NULL);

    fdb_assert("Reference count not decremented", fh2->inodeInfo->refCount == 1);
    fdb_close_file(fh2);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_assert("Inode info list is not null", inodeInfoList == NULL);

    fdb_passed;
}

void test_close_file() {
    FileHandle *fh1;
    FileHandle *fh2;
    stat_t st;

    int fd1;

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);

    fdb_open_file_rdwr(TEMPFILENAME, &fh1);
    fdb_open_file_rdwr(TEMPFILENAME, &fh2);

    fdb_assert("File handle not created", fh1);
    fdb_assert("File handle not created", fh2);
    fdb_assert("Do not have same inode", fh1->inodeInfo && fh1->inodeInfo == fh2->inodeInfo);
    fdb_assert("Inode unused is not null", fh1->inodeInfo->unusedFiles == NULL);

    fd1 = fh1->fd;

    /* set a fake lock on the inode */
    fh1->inodeInfo->lockCount++;

    fdb_assert("Ref count is decremented", fh2->inodeInfo->refCount == 2);

    /* fd should not be closed */
    fdb_close_file(fh1);
    fdb_assert("Inode unused is null", fh2->inodeInfo->unusedFiles != NULL);
    fdb_assert("Ref count is decremented", fh2->inodeInfo->refCount == 1);

    /* The file descriptor should still be open */
    fdb_assert("File descriptor was closed", fstat(fd1, &st) != -1);
    fdb_assert("Unused file handle's fd not set properly", fh1->inodeInfo->unusedFiles->fd == fd1);

    /* remove fake lock */
    fh2->inodeInfo->lockCount--;
    fdb_inodeinfo_close_unused_files(fh2->inodeInfo);

    fdb_assert("Inode unused files not null", fh2->inodeInfo->unusedFiles == NULL);

    fdb_close_file(fh2);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_truncate() {
    remove(TEMPFILENAME);

    FileHandle *fh;
    off_t size;
    unsigned char* bytes = (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZ";



    fdb_assert("Could not create file", fdb_create_file(TEMPFILENAME, &fh) == FABRICDB_OK);

    fdb_assert("Size failed", fdb_file_size(fh, &size) == FABRICDB_OK);
    fdb_assert("File not zero", size == 0);

    fdb_assert("Could not write", fdb_write(fh, bytes, 0, 26) == FABRICDB_OK);
    fdb_assert("Size failed", fdb_file_size(fh, &size) == FABRICDB_OK);
    fdb_assert("File not right size", size == 26);

    fdb_assert("Truncate failed to extend file", fdb_truncate_file(fh, 200) == FABRICDB_OK);
    fdb_assert("Size failed", fdb_file_size(fh, &size) == FABRICDB_OK);
    fdb_assert("File not right size", size == 200);

    fdb_assert("Truncate failed to extend file", fdb_truncate_file(fh, 0) == FABRICDB_OK);
    fdb_assert("Size failed", fdb_file_size(fh, &size) == FABRICDB_OK);
    fdb_assert("File not right size", size == 0);

    fdb_close_file(fh);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_read() {
    remove(TEMPFILENAME);

    FileHandle *fh;
    off_t size;
    unsigned char* bytes = (unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned char buff[10];
    int fd;


    fdb_assert("Could not create file", fdb_create_file(TEMPFILENAME, &fh) == FABRICDB_OK);

    fdb_assert("Size failed", fdb_file_size(fh, &size) == FABRICDB_OK);
    fdb_assert("File not zero", size == 0);

    fdb_assert("Could not write", fdb_write(fh, bytes, 0, 26) == FABRICDB_OK);
    fdb_assert("Size failed", fdb_file_size(fh, &size) == FABRICDB_OK);
    fdb_assert("File not right size", size == 26);

    fdb_assert("Could not read", fdb_read(fh, buff, 0, 10) == FABRICDB_OK);
    fdb_assert("Read incorrectly", memcmp(bytes, buff, 10) == 0);

    fdb_assert("Could not read", fdb_read(fh, buff, 13, 10) == FABRICDB_OK);
    fdb_assert("Read incorrectly", memcmp(bytes + 13, buff, 10) == 0);

    fdb_assert("Failed to return correct error", fdb_read(fh, buff, 20, 10) == FABRICDB_ESHORTREAD);

    fd = fh->fd;
    fh->fd = -1;

    fdb_assert("Failed to return error code", fdb_read(fh, buff, 0, 10) != FABRICDB_OK);

    fh->fd = fd;
    fdb_close_file(fh);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_sync() {
    FileHandle *fh;
    int fd;

    remove(TEMPFILENAME);
    fdb_assert("Could not create file", fdb_create_file(TEMPFILENAME, &fh) == FABRICDB_OK);

    fdb_assert("Sync failed", fdb_sync(fh) == FABRICDB_OK);

    fd = fh->fd;
    fh->fd = -1;
    fdb_assert("Failed to return error code", fdb_sync(fh) != FABRICDB_OK);
    fh->fd = fd;

    fdb_close_file(fh);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

static int get_lock(int fd, off_t start, int *out){
    struct flock lock;
    pid_t pid;
    int pfds[2];
    int rv;

    /* This keeps the test name from printing multiple times */
    fflush(stdout);

    pipe(pfds);

    /* A fork and a pipe
       This seems to be the only reliable way to check
       whether or not a file lock actually exists from
       the process that created it.  Weird... */
    switch(pid = fork()) {
        case -1:
            *out = -1;
            return -1;
        case 0:
            close(pfds[0]);
            lock.l_whence = SEEK_SET;
            lock.l_start = start;
            lock.l_len = 1;
            lock.l_type = F_WRLCK;
            if (fcntl(fd, F_GETLK, &lock) == -1) {
                rv = -1;
                write(pfds[1], &rv, sizeof(int));
                exit(1);
            }
            // printf("CHILD PROCESS LTYPE: %d\n", lock.l_type);
            rv = lock.l_type;
            write(pfds[1], &rv, sizeof(int));
            exit(0);
        default:
            close(pfds[1]);
            read(pfds[0], &rv, sizeof(int));
            wait(&pid);
            // printf("RETURN VALUE WAS: %d AND F_UNLCK IS: %d\n", rv, F_UNLCK);
            *out = rv;
            return FABRICDB_OK;
    }


}

void test_acquire_shared_lock_1() {
    int lockType;
    FileHandle *fh;
    int rc;

    /* open the file */
    fdb_assert("Could not open file", fdb_open_file_rdwr(TEMPFILENAME, &fh) == FABRICDB_OK);

    /* ensure the file has no lock right now */
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 0);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 0);
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File is already locked", lockType == F_UNLCK);

    /* acquire the lock */
    rc = fdb_acquire_shared_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* ensure the file is actually locked */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File is already locked", lockType == F_RDLCK);


    /* get rid of the lock */
    rc = fdb_unlock(fh);
    fdb_assert("Unlock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 0);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 0);

    /* ensure the file is in fact unlocked */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File lock was not released", lockType == F_UNLCK);

    /* simulate the inode having a shared lock */
    fh->inodeInfo->lockLevel = FDB_SHARED_LOCK;
    fh->inodeInfo->lockCount = 1;
    fh->inodeInfo->sharedLockCount = 1;
    rc = fdb_acquire_shared_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 2);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 2);

    /* this should skip the actual unlocking */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File lock was set when it shouldn't have been", lockType == F_UNLCK);

    rc = fdb_unlock(fh);
    fdb_assert("Unlock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* simulate the inode having a reserved lock */
    fh->inodeInfo->lockLevel = FDB_RESERVED_LOCK;
    fh->inodeInfo->lockCount = 1;
    fh->inodeInfo->sharedLockCount = 1;
    rc = fdb_acquire_shared_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_RESERVED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 2);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 2);

    /* this should skip the actual unlocking */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File lock was set when it shouldn't have been", lockType == F_UNLCK);

    rc = fdb_unlock(fh);
    fdb_assert("Unlock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_RESERVED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* simulate the inode having a pending lock */
    fh->inodeInfo->lockLevel = FDB_PENDING_LOCK;
    fh->inodeInfo->lockCount = 1;
    fh->inodeInfo->sharedLockCount = 1;

    rc = fdb_acquire_shared_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_BUSY);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_PENDING_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* simulate the inode having an exlusive lock */
    fh->inodeInfo->lockLevel = FDB_EXCLUSIVE_LOCK;
    fh->inodeInfo->lockCount = 1;
    fh->inodeInfo->sharedLockCount = 1;

    rc = fdb_acquire_shared_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_BUSY);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_EXCLUSIVE_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* reset inode to where it should be */
    fh->inodeInfo->lockLevel = FDB_NO_LOCK;
    fh->inodeInfo->lockCount = 0;
    fh->inodeInfo->sharedLockCount = 0;

    fdb_close_file(fh);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_acquire_shared_lock_2() {
    pid_t pid;
    int pfds[2];
    FileHandle *fh;
    int rv;

    /* This keeps the test name from printing multiple times */
    fflush(stdout);

    pipe(pfds);

    pid = fork();
    if (pid == -1) {
        fdb_assert("Fork failed", 0);
    } else if (!pid) {
        /* child process */
        fdb_open_file_rdwr(TEMPFILENAME, &fh);

        fdb_acquire_shared_lock(fh);
        rv = 1;
        write(pfds[1], &rv, sizeof(int));

        fdb_acquire_exclusive_lock(fh);
        rv = 2;
        write(pfds[1], &rv, sizeof(int));


        sleep(1);
        fdb_unlock(fh);

        fdb_close_file(fh);
        exit(0);
    } else {
        /* parent process */
        fdb_open_file_rdwr(TEMPFILENAME, &fh);
        read(pfds[0], &rv, sizeof(int));
        fdb_assert("Wrong read value", rv == 1);

        read(pfds[0], &rv, sizeof(int));
        fdb_assert("Wrong read value", rv == 2);

        fdb_assert("Cross pid memory sharing?", fh->inodeInfo->lockLevel == FDB_NO_LOCK);
        fdb_assert("Should be busy", fdb_acquire_shared_lock(fh) == FABRICDB_BUSY);
        fdb_assert("Lock level was set", fh->inodeInfo->lockLevel == FDB_NO_LOCK);
        fdb_assert("Lock count was incremented", fh->inodeInfo->lockCount == 0);
        fdb_assert("Shared lock count was incremented", fh->inodeInfo->sharedLockCount == 0);

        fdb_unlock(fh);
        fdb_close_file(fh);

        wait(&pid);
    }

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_acquire_reserved_lock_1() {
    int lockType;
    FileHandle *fh;
    int rc;

    /* open the file */
    fdb_assert("Could not open file", fdb_open_file_rdwr(TEMPFILENAME, &fh) == FABRICDB_OK);

    /* ensure the file has no lock right now */
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 0);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 0);
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File is already locked", lockType == F_UNLCK);

    /* acquire a shared lock first */
    rc = fdb_acquire_shared_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* ensure the file is actually locked */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File is not read locked", lockType == F_RDLCK);

    /* acquire a reserved lock */
    rc = fdb_acquire_reserved_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_RESERVED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_RESERVED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* ensure the file is actually locked */
    rc = get_lock(fh->fd, RESERVED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("Byte is not write locked", lockType == F_WRLCK);

    /* downgrade the lock */
    rc = fdb_downgrade_lock(fh);
    fdb_assert("Downgrad failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* ensure the file is still read locked */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("Shared file lock was released", lockType == F_RDLCK);

    /* ensure the file is no longer reserved locked */
    rc = get_lock(fh->fd, RESERVED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("Exclusive file lock was not released", lockType == F_UNLCK);

    /* downgrade the lock */
    rc = fdb_downgrade_lock(fh);
    fdb_assert("Unlock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* simulate the inode having a reserved lock */
    fh->inodeInfo->lockLevel = FDB_RESERVED_LOCK;
    fh->inodeInfo->lockCount = 2;
    fh->inodeInfo->sharedLockCount = 2;

    rc = fdb_acquire_reserved_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_BUSY);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_RESERVED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 2);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 2);

    /* this should skip the actual unlocking */
    rc = get_lock(fh->fd, RESERVED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File lock was set when it shouldn't have been", lockType == F_UNLCK);

    /* simulate the inode having a pending lock */
    fh->inodeInfo->lockLevel = FDB_PENDING_LOCK;
    fh->inodeInfo->lockCount = 2;
    fh->inodeInfo->sharedLockCount = 2;

    rc = fdb_acquire_reserved_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_BUSY);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_PENDING_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 2);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 2);

    /* reset inode to where it should be */
    fh->inodeInfo->lockLevel = FDB_SHARED_LOCK;
    fh->inodeInfo->lockCount = 1;
    fh->inodeInfo->sharedLockCount = 1;

    rc = fdb_unlock(fh);
    fdb_assert("Unlock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 0);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 0);

    fdb_close_file(fh);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_acquire_reserved_lock_2() {
    pid_t pid;
    int pfds[2];
    FileHandle *fh;
    int rv;

    /* This keeps the test name from printing multiple times */
    fflush(stdout);

    pipe(pfds);

    pid = fork();
    if (pid == -1) {
        fdb_assert("Fork failed", 0);
    } else if (!pid) {
        /* child process */
        fdb_open_file_rdwr(TEMPFILENAME, &fh);

        fdb_acquire_shared_lock(fh);
        rv = 1;
        write(pfds[1], &rv, sizeof(int));

        sleep(1);
        fdb_acquire_reserved_lock(fh);
        rv = 2;
        write(pfds[1], &rv, sizeof(int));


        sleep(1);
        fdb_unlock(fh);

        fdb_close_file(fh);
        exit(0);
    } else {
        /* parent process */
        fdb_open_file_rdwr(TEMPFILENAME, &fh);
        fdb_assert("Coult not get shared lock", fdb_acquire_shared_lock(fh) == FABRICDB_OK);
        read(pfds[0], &rv, sizeof(int));
        fdb_assert("Wrong read value", rv == 1);

        read(pfds[0], &rv, sizeof(int));
        fdb_assert("Wrong read value", rv == 2);

        fdb_assert("Cross pid memory sharing?", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
        fdb_assert("Should be busy", fdb_acquire_reserved_lock(fh) == FABRICDB_BUSY);
        fdb_assert("Lock level was set", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
        fdb_assert("Lock count was incremented", fh->inodeInfo->lockCount == 1);
        fdb_assert("Shared lock count was incremented", fh->inodeInfo->sharedLockCount == 1);

        fdb_unlock(fh);
        fdb_close_file(fh);

        wait(&pid);
    }

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_acquire_exclusive_lock_1() {
    int lockType;
    FileHandle *fh;
    int rc;

    /* open the file */
    fdb_assert("Could not open file", fdb_open_file_rdwr(TEMPFILENAME, &fh) == FABRICDB_OK);

    /* ensure the file has no lock right now */
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 0);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 0);
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File is already locked", lockType == F_UNLCK);

    /* acquire a shared lock first */
    rc = fdb_acquire_shared_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* ensure the file is actually locked */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File is not read locked", lockType == F_RDLCK);

    /* acquire a reserved lock */
    rc = fdb_acquire_exclusive_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_EXCLUSIVE_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_EXCLUSIVE_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* ensure the file is actually locked */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("Byte is not write locked", lockType == F_WRLCK);

    /* downgrade the lock */
    rc = fdb_downgrade_lock(fh);
    fdb_assert("Downgrad failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* ensure the file is still read locked */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("Shared file lock was released", lockType == F_RDLCK);

    /* simulate the inode having a reserved lock */
    fh->inodeInfo->lockLevel = FDB_RESERVED_LOCK;
    fh->inodeInfo->lockCount = 2;
    fh->inodeInfo->sharedLockCount = 2;

    rc = fdb_acquire_exclusive_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_BUSY);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_RESERVED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 2);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 2);

    /* this should skip the actual unlocking */
    rc = get_lock(fh->fd, SHARED_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("File lock was set when it shouldn't have been", lockType == F_RDLCK);

    /* simulate the inode having an additional shared lock */
    fh->inodeInfo->lockLevel = FDB_SHARED_LOCK;
    fh->inodeInfo->lockCount = 2;
    fh->inodeInfo->sharedLockCount = 2;

    rc = fdb_acquire_exclusive_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_BUSY);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_PENDING_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_PENDING_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 2);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 2);

    /* ensure the file is actually pending locked */
    rc = get_lock(fh->fd, PENDING_BYTE, &lockType);
    fdb_assert("Get lock failed", rc == FABRICDB_OK);
    fdb_assert("Shared file lock was released", lockType == F_WRLCK);

    /* simulate the inode losing the additonal shared lock */
    fh->inodeInfo->lockLevel = FDB_SHARED_LOCK;
    fh->inodeInfo->lockCount = 1;
    fh->inodeInfo->sharedLockCount = 1;

    rc = fdb_acquire_exclusive_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_EXCLUSIVE_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_EXCLUSIVE_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* downgrade the lock */
    rc = fdb_downgrade_lock(fh);
    fdb_assert("Downgrad failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);


    /* simulate the inode having a pending lock */
    fh->inodeInfo->lockLevel = FDB_PENDING_LOCK;
    fh->inodeInfo->lockCount = 2;
    fh->inodeInfo->sharedLockCount = 2;

    rc = fdb_acquire_exclusive_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_BUSY);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_SHARED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_PENDING_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 2);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 2);

    /* reset inode to where it should be */
    fh->inodeInfo->lockLevel = FDB_SHARED_LOCK;
    fh->inodeInfo->lockCount = 1;
    fh->inodeInfo->sharedLockCount = 1;

    /* acquire a reserved lock */
    rc = fdb_acquire_reserved_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_RESERVED_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_RESERVED_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    /* should be able to upgrade to exclusive from reserved */
    rc = fdb_acquire_exclusive_lock(fh);
    fdb_assert("Set lock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong", fh->lockLevel == FDB_EXCLUSIVE_LOCK);
    fdb_assert("Lock level wrong for inode", fh->inodeInfo->lockLevel == FDB_EXCLUSIVE_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 1);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 1);

    rc = fdb_unlock(fh);
    fdb_assert("Unlock failed", rc == FABRICDB_OK);
    fdb_assert("Lock level wrong for inode", fh->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->lockLevel == FDB_NO_LOCK);
    fdb_assert("Shared lock count is wrong", fh->inodeInfo->sharedLockCount == 0);
    fdb_assert("Lock count is wrong", fh->inodeInfo->lockCount == 0);

    fdb_close_file(fh);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_acquire_exclusive_lock_2() {
    pid_t pid;
    int pfds[2];
    FileHandle *fh;
    int rv;

    /* This keeps the test name from printing multiple times */
    fflush(stdout);

    pipe(pfds);

    pid = fork();
    if (pid == -1) {
        fdb_assert("Fork failed", 0);
    } else if (!pid) {
        /* child process */
        fdb_open_file_rdwr(TEMPFILENAME, &fh);

        fdb_acquire_shared_lock(fh);
        rv = 1;
        write(pfds[1], &rv, sizeof(int));

        sleep(1);
        fdb_unlock(fh);

        fdb_close_file(fh);
        exit(0);
    } else {
        /* parent process */
        fdb_open_file_rdwr(TEMPFILENAME, &fh);
        fdb_assert("Coult not get shared lock", fdb_acquire_shared_lock(fh) == FABRICDB_OK);
        read(pfds[0], &rv, sizeof(int));
        fdb_assert("Wrong read value", rv == 1);

        fdb_assert("Cross pid memory sharing?", fh->inodeInfo->lockLevel == FDB_SHARED_LOCK);
        fdb_assert("Should be busy", fdb_acquire_exclusive_lock(fh) == FABRICDB_BUSY);
        fdb_assert("Lock level was set", fh->inodeInfo->lockLevel == FDB_PENDING_LOCK);
        fdb_assert("Lock count was incremented", fh->inodeInfo->lockCount == 1);
        fdb_assert("Shared lock count was incremented", fh->inodeInfo->sharedLockCount == 1);

        fdb_unlock(fh);
        fdb_close_file(fh);

        wait(&pid);
    }

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_ioerror_from_errno() {
    errno = EACCES;
    fdb_assert("EACCESS returned wrong value", fdb_ioerror_from_errno() == FABRICDB_EACCES);
    errno = EEXIST;
    fdb_assert("EEXIST returned wrong value", fdb_ioerror_from_errno() == FABRICDB_EEXIST);
    errno = EISDIR;
    fdb_assert("EISDIR returned wrong value", fdb_ioerror_from_errno() == FABRICDB_EISDIR);
    errno = ELOOP;
    fdb_assert("ELOOP returned wrong value", fdb_ioerror_from_errno() == FABRICDB_ELOOP);
    errno = EMFILE;
    fdb_assert("EMFILE returned wrong value", fdb_ioerror_from_errno() == FABRICDB_EMFILE);
    errno = ENAMETOOLONG;
    fdb_assert("ENAMETOOLONG returned wrong value", fdb_ioerror_from_errno() == FABRICDB_ENAMETOOLONG);
    errno = ENFILE;
    fdb_assert("ENFILE returned wrong value", fdb_ioerror_from_errno() == FABRICDB_ENFILE);
    errno = ENOENT;
    fdb_assert("ENOENT returned wrong value", fdb_ioerror_from_errno() == FABRICDB_ENOENT);
    errno = ENOSPC;
    fdb_assert("ENOSPC returned wrong value", fdb_ioerror_from_errno() == FABRICDB_ENOSPC);
    errno = ENOTDIR;
    fdb_assert("ENOTDIR returned wrong value", fdb_ioerror_from_errno() == FABRICDB_ENOTDIR);
    errno = EOVERFLOW;
    fdb_assert("EOVERFLOW returned wrong value", fdb_ioerror_from_errno() == FABRICDB_EOVERFLOW);
    errno = EINVAL;
    fdb_assert("EINVAL returned wrong value", fdb_ioerror_from_errno() == FABRICDB_EINVAL);
    errno = EFBIG;
    fdb_assert("EFBIG returned wrong value", fdb_ioerror_from_errno() == FABRICDB_EFBIG);
    errno = EBADF;
    fdb_assert("EBADF returned wrong value", fdb_ioerror_from_errno() == FABRICDB_EBADF);
    errno = ENOBUFS;
    fdb_assert("ENOBUFS returned wrong value", fdb_ioerror_from_errno() == FABRICDB_ENOBUFS);
    errno = ENOMEM;
    fdb_assert("ENOMEN returned wrong value", fdb_ioerror_from_errno() == FABRICDB_ENOMEM);

    fdb_passed;
}

void test_os_unix() {
    fdb_runtest("Create File", test_create_file);
    fdb_runtest("Open File Read-Write", test_open_file_rdwr);
    fdb_runtest("Open File Read Only", test_open_file_rdonly);
    fdb_runtest("Fetch Inode Info", test_inodeinfo_fetch);
    fdb_runtest("Close File", test_close_file);
    fdb_runtest("Truncate", test_truncate);
    fdb_runtest("Read", test_read);
    fdb_runtest("Sync", test_sync);
    fdb_runtest("Acquire shared lock 1", test_acquire_shared_lock_1);
    fdb_runtest("Acquire shared lock 2", test_acquire_shared_lock_2);
    fdb_runtest("Acquire reserved lock 1",test_acquire_reserved_lock_1);
    fdb_runtest("Acquire reserved lock 2", test_acquire_reserved_lock_2);
    fdb_runtest("Acquire exclusive lock 1", test_acquire_exclusive_lock_1);
    fdb_runtest("Acquire exclusive lock 2", test_acquire_exclusive_lock_2);
    fdb_runtest("IO Error from errno", test_ioerror_from_errno);
}
