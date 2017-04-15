/*****************************************************************
 * FabricDB Library Vertex Implementation
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
 *     Implements operations on the Vertex datatype.
 *
 ******************************************************************/

#include <stdlib.h>
#include <string.h>

#include "vertex.h"
#include "byteorder.h"

void fdb_vertex_load(Vertex* vert, uint32_t id, uint8_t* source) {
    vert->id = id;
    vert->symbolId = letohu16(*((uint16_t*) (source + FDB_VERTEX_SYMBOLID_OFFSET)));
    vert->firstOutEdgeId = letohu32(*((uint32_t*) (source + FDB_VERTEX_FIRSTOUTEDGEID_OFFSET)));
    vert->firstInEdgeId = letohu32(*((uint32_t*) (source + FDB_VERTEX_FIRSTINEDGEID_OFFSET)));

    fdb_property_load(&vert->value, source + FDB_VERTEX_VALUE_OFFSET);
}

void fdb_vertex_unload(Vertex* vert, uint8_t* dest) {
    uint16_t symbolId = htoleu16(vert->symbolId);
    uint32_t firstOutEdgeId = htoleu32(vert->firstOutEdgeId);
    uint32_t firstInEdgeId = htoleu32(vert->firstInEdgeId);

    memcpy(dest + FDB_VERTEX_SYMBOLID_OFFSET, &symbolId, 2);
    memcpy(dest + FDB_VERTEX_FIRSTOUTEDGEID_OFFSET, &firstOutEdgeId, 4);
    memcpy(dest + FDB_VERTEX_FIRSTINEDGEID_OFFSET, &firstInEdgeId, 4);

    fdb_property_unload(&vert->value, dest + FDB_VERTEX_VALUE_OFFSET);
}

#ifdef FABRICDB_TESTING
#include "../test/test_vertex.c"
#endif
