/*****************************************************************
 * FabricDB Library Main File
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: January 28, 2016
 * Modified: January 29, 2016
 * Author: Mark Wardle
 * Description:
 *     This file implements the top-level functionality for FabricDB.
 *
 ******************************************************************/

#include <string.h>
#include <stdint.h>
#include "fabric.h"
#include "mem.h"

/**
 * FabricDB's 16 byte header string
 * Must be present as the first 16 bytes of the database file
 */
const char* FABRICDB_HEADER_STRING = "FabricDB vers 1"

typedef struct fabricdb {
    uint16_t page_size;
    uint8_t write_version;
    uint8_t read_version;
    uint8_t page_reserved_space;
} fabricdb;

int fabricdb_open(const char *dbname, fabricdb** dbptr) {

    fabricdb *db;
    int rc;

    if (dbptr == 0) {
        return FABRICDB_EMISUSE_NULLPTR;
    }

    *dbptr = 0;

    /* Allocate memory for the database */
    db = fdbmalloc(sizeof(fabricdb));
    if(db == 0) {
	       return FABRICDB_ENOMEM;
    }
}
