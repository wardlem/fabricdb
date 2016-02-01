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
 * Modified: January 29, 2016
 * Author: Mark Wardle
 * Description:
 *     This file declares the operating system abstraction functionality
 *
 ******************************************************************/

#ifndef __FABRICDB_OS_H
#define __FABRICDB_OS_H

#include <stdint.h>

typedef struct FileHandle FileHandle;

int fabricdb_open_file_rdwr(const char *filepath, FileHandle **fhp);
int fabricdb_open_file_rdonly(const char *filepath, FileHandle **fhp);
int fabricdb_create_file(const char *filepath, FileHandle **fhp);
int fabricdb_close_file(FileHandle *fh);
int fabricdb_truncate_file(int64_t size, FileHandle *fh);
int fabricdb_file_size(int64_t out, FileHandle *fh);
int fabricdb_read(uint64_t offset, uint32_t num_bytes, uint8_t *dest, FileHandle *fh);
int fabricdb_write(uint64_t, offset, uint32_t num_bytes, uint8_t *content, FileHandle *fh);
int fabricdb_read8(uint64_t offset, uint8_t *dest, FileHandle *fh);
int fabricdb_read16(uint64_t offset, uint16_t *dest, FileHandle *fh);
int fabricdb_read32(uint64_t offset, uint32_t *dest, FileHandle *fh);
int fabricdb_read64(uint64_t offset, uint64_t *dest, FileHandle *fh);

int fabricdb_get_shared_lock(FileHandle *fh);
int fabricdb_get_reserved_lock(FileHandle *fh);
int fabricb_get_exclusive_lock(FileHandle *fh);
int fabricb_check_exclusive_lock(FileHandle *fh);
int fabricdb_downgrade_lock(FileHandle *fh);
int fabricdb_unlock(FileHandle *fh);



#endif /* __FABRICDB_OS_H */