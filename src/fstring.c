/*****************************************************************
 * FabricDB Library FString Implementation
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
 *     Implements operations on the FString datatype.
 *
 ******************************************************************/

#include <stdint.h>
#include <string.h>

#include "fstring.h"
#include "byteorder.h"
#include "mem.h"
#include "fabric.h"

void fdb_fstring_load(FString* fstring, uint64_t id, uint8_t* source) {
    fstring->id = id;
    fstring->size = letohu32(*((uint32_t*)(source + FDB_FSTRING_SIZE_OFFSET)));
    fstring->data = source + FDB_FSTRING_DATA_OFFSET;
}

void fdb_fstring_unload(FString* fstring, uint8_t* dest) {
    *((uint32_t*)(dest + FDB_FSTRING_SIZE_OFFSET)) = htoleu32(fstring->size);
    memcpy(dest + FDB_FSTRING_DATA_OFFSET, fstring->data, fstring->size);

    // TODO: fill the rest of chunk with null bytes
}

int fdb_fstring_tocstring(FString* fstring, char** out) {
    char* cstring = fabricdb_malloc((size_t)fstring->size + 1);

    if (cstring == NULL) {
        return FABRICDB_ENOMEM;
    }

    memcpy(cstring, (char*)fstring->data, fstring->size);
    cstring[fstring->size] = '\0';
    *out = cstring;

    return FABRICDB_OK;
}


#ifdef FABRICDB_TESTING
#include "../test/test_fstring.c"
#endif
