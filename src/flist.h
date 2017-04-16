/*****************************************************************
 * FabricDB Library FList Header
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: April 15, 2017
 * Modified: April 15, 2017
 * Author: Mark Wardle
 * Description:
 *     Defines the FList datatype and operations on it.
 *
 ******************************************************************/

#ifndef __FABRICDB_FLIST_H
#define __FABRICDB_FLIST_H

#include <stdint.h>

#include "property.h"

/******************************************************
 * FLIST FORMAT
 *
 * +-----+------+------------------------------------
 * | pos | size | description
 * +-----+------+------------------------------------
 * |   0 |    9 | entry (embedded property)
 * |   9 |    8 | nextEntryId
 * +-----+------+------------------------------------
 *
 ******************************************************/

#define FDB_FLIST_ENTRY_OFFSET 0
#define FDB_FLIST_NEXTENTRYID_OFFSET 9
#define FDB_FLIST_DISKSIZE 17

typedef struct FList {
    uint64_t id;
    Property entry;
    uint64_t nextEntryId;
} FList;

void fdb_flist_load(FList* list, uint64_t id, uint8_t* source);
void fdb_flist_unload(FList* list, uint8_t* dest);


#endif /* __FABRICDB_FLIST_H */
