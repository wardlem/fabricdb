/*****************************************************************
 * FabricDB Library Document Implementation
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
 *     Implements operatons on the Document datatype.
 *
 ******************************************************************/

#include <stdint.h>
#include <string.h>

#include "document.h"
#include "byteorder.h"

void fdb_document_load(Document* doc, uint64_t id, uint8_t* source) {
    doc->id = id;
    fdb_labeledproperty_load(&doc->entry, source + FDB_DOCUMENT_ENTRY_OFFSET);
    doc->nextEntryId = letohu64(*((uint64_t*)(source + FDB_DOCUMENT_NEXTENTRYID_OFFSET)));
}

void fdb_document_unload(Document* doc, uint8_t* dest) {
    uint64_t nextEntryId = htoleu64(doc->nextEntryId);
    fdb_labeledproperty_unload(&doc->entry, dest + FDB_DOCUMENT_ENTRY_OFFSET);
    memcpy(dest + FDB_DOCUMENT_NEXTENTRYID_OFFSET, &nextEntryId, 8);
}

#ifdef FABRICDB_TESTING
#include "../test/test_document.c"
#endif
