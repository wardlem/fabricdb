/*****************************************************************
 * FabricDB Library Document Header
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

#ifndef __FABRICDB_DOCUMENT_H
#define __FABRICDB_DOCUMENT_H

#include <stdint.h>

#include "property.h"

/******************************************************
 * FLIST FORMAT
 *
 * +-----+------+------------------------------------
 * | pos | size | description
 * +-----+------+------------------------------------
 * |   0 |   13 | entry (labeled embedded property)
 * |  13 |    8 | nextEntryId
 * +-----+------+------------------------------------
 *
 ******************************************************/

#define FDB_DOCUMENT_ENTRY_OFFSET 0
#define FDB_DOCUMENT_NEXTENTRYID_OFFSET 13
#define FDB_DOCUMENT_DISKSIZE 21

typedef struct Document {
    uint64_t id;
    LabeledProperty entry;
    uint64_t nextEntryId;
} Document;

void fdb_document_load(Document* doc, uint64_t id, uint8_t* source);
void fdb_document_unload(Document* doc, uint8_t* dest);

#endif /* __FABRICDB_DOCUMENT_H */
