/*****************************************************************
 * FabricDB Library FList Implementation
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: April 16, 2017
 * Modified: April 16, 2017
 * Author: Mark Wardle
 * Description:
 *     Implements operatons on the FList datatype.
 *
 ******************************************************************/

#include <stdint.h>
#include <string.h>

#include "flist.h"
#include "byteorder.h"

void fdb_flist_load(FList* list, uint64_t id, uint8_t* source) {
    list->id = id;
    fdb_property_load(&list->entry, source + FDB_FLIST_ENTRY_OFFSET);
    list->nextEntryId = letohu64(*((uint64_t*)(source + FDB_FLIST_NEXTENTRYID_OFFSET)));
}

void fdb_flist_unload(FList* list, uint8_t* dest) {
    uint64_t nextEntryId = htoleu64(list->nextEntryId);
    fdb_property_unload(&list->entry, dest + FDB_FLIST_ENTRY_OFFSET);
    memcpy(dest + FDB_FLIST_NEXTENTRYID_OFFSET, &nextEntryId, 8);
}

#ifdef FABRICDB_TESTING
#include "../test/test_flist.c"
#endif
