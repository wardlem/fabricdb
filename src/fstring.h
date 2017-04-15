/*****************************************************************
 * FabricDB Library FString Header
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: November 13, 2016
 * Modified: April 15, 2017
 * Author: Mark Wardle
 * Description:
 *     Defines the FString datatype and operations for them.
 *
 ******************************************************************/

#ifndef __FABRICDB_FSTRING_H
#define __FABRICDB_FSTRING_H

#include <stdint.h>

/******************************************************
 * FSTRING FORMAT
 *
 * +-----+------+------------------------------------
 * | pos | size | description
 * +-----+------+------------------------------------
 * |   0 |    4 | string size (in bytes)
 * |   4 |  28+ | data (in 32 byte chunks)
 * +-----+------+------------------------------------
 *
 ******************************************************/

#define FDB_FSTRING_SIZE_OFFSET 0
#define FDB_FSTRING_DATA_OFFSET 4
#define FDB_FSTRING_CHUNKSIZE 32

typedef struct FString {
    uint64_t id;
    uint32_t size;
    uint8_t* data;
} FString;

void fdb_fstring_load(FString* fstring, uint64_t id, uint8_t* source);
void fdb_fstring_unload(FString* fstring, uint8_t* dest);
int fdb_fstring_tocstring(FString* fstring, char** out);

#endif /* __FABRICDB_FSTRING_H */
