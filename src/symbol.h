/*****************************************************************
 * FabricDB Library Symbol Header
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: July 9, 2016
 * Modified: July 9, 2016
 * Author: Mark Wardle
 * Description:
 *     Declares the property datatype and operations on symbols.
 *
 ******************************************************************/

#ifndef __FABRICDB_SYMBOL_H
#define __FABRICDB_SYMBOL_H

#include <stdint.h>

/******************************************************
 * SYMBOL FORMAT
 *
 * +-----+------+------------------------------------
 * | pos | size | description
 * +-----+------+------------------------------------
 * |   0 |    4 | reference count
 * |   4 |    8 | string id
 * +-----+------+------------------------------------
 *
 * A symbol is a reference counted object that can be
 * converted to and from a string.  It used for vertex
 * and edge labels as well as property keys in documents.
 * It can also be used as a property value for faster
 * equality checks than strings.
 *
 ******************************************************/

#define FDB_SYMBOL_REFCOUNT_OFFSET 0
#define FDB_SYMBOL_STRINGID_OFFSET 4
#define FDB_SYMBOL_DISKSIZE 12

typedef struct Symbol {
    uint32_t id;
    uint32_t refCount;
    uint64_t stringId;
    void* stringRef;   /* reference to in memory fstring object */
} Symbol;

void fdb_symbol_load(Symbol* symbol, uint32_t id, uint8_t* source);
void fdb_symbol_unload(Symbol* symbol, uint8_t* dest);

#endif /* __FABRICDB_SYMBOL_H */
