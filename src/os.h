/*****************************************************************
 * FabricDB Library OS Abstraction Header.
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: January 29, 2016
 * Modified: July 1, 2016
 * Author: Mark Wardle
 * Description:
 *     This file declares the operating system abstraction functionality
 *
 ******************************************************************/

#ifndef __FABRICDB_OS_H
#define __FABRICDB_OS_H

#include <stdint.h>

typedef struct FileHandle FileHandle;

#define FDB_NO_LOCK 0
#define FDB_SHARED_LOCK 1
#define FDB_RESERVED_LOCK 2
#define FDB_PENDING_LOCK 3
#define FDB_EXCLUSIVE_LOCK 4

int fdb_open_file_rdwr(const char *filepath, FileHandle **fhp);
int fdb_open_file_rdonly(const char *filepath, FileHandle **fhp);
int fdb_create_file(const char *filepath, FileHandle **fhp);
int fdb_close_file(FileHandle *fh);
int fdb_truncate_file(FileHandle *fh, off_t size);
int fdb_file_size(FileHandle *fh, off_t *out);
int fdb_read(FileHandle *fh, uint8_t *dest, off_t offset, size_t num_bytes);
int fdb_write(FileHandle *fh, uint8_t *content, off_t offset, size_t num_bytes);
int fdb_sync(FileHandle *fh);

int fdb_acquire_shared_lock(FileHandle *fh);
int fdb_acquire_reserved_lock(FileHandle *fh);
int fdb_acquire_exclusive_lock(FileHandle *fh);
int fdb_unlock(FileHandle *fh);
int fdb_downgrade_lock(FileHandle *fh);
int fdb_get_lock_level(FileHandle *fh);


#endif /* __FABRICDB_OS_H */
