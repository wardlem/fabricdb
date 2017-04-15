/*****************************************************************
 * FabricDB Library Symbol Implementation
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
 *     Defines operations on symbols.
 *
 ******************************************************************/

#include <stdint.h>
#include <string.h>

#include "symbol.h"
#include "byteorder.h"

void fdb_symbol_load(Symbol* symbol, uint32_t id, uint8_t* source) {
    symbol->id = id;
    symbol->refCount = letohu32(*((uint32_t*)(source + FDB_SYMBOL_REFCOUNT_OFFSET)));
    symbol->stringId = letohu64(*((uint64_t*)(source + FDB_SYMBOL_STRINGID_OFFSET)));
    symbol->stringRef = NULL;
}

void fdb_symbol_unload(Symbol* symbol, uint8_t* dest) {
    *((uint32_t*)(dest + FDB_SYMBOL_REFCOUNT_OFFSET)) = htoleu32(symbol->refCount);
    *((uint64_t*)(dest + FDB_SYMBOL_STRINGID_OFFSET)) = htoleu64(symbol->stringId);
}

#ifdef FABRICDB_TESTING
#include "../test/test_symbol.c"
#endif
