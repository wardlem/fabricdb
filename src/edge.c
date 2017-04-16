/*****************************************************************
 * FabricDB Library Edge Implementation
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
 *     Implements operations on the Edge datatype.
 *
 ******************************************************************/

#include <stdlib.h>
#include <string.h>

#include "edge.h"
#include "byteorder.h"

void fdb_edge_load(Edge* edge, uint32_t id, uint8_t* source) {
    edge->id = id;
    edge->symbolId = letohu32(*((uint32_t*) (source + FDB_EDGE_SYMBOLID_OFFSET)));
    edge->fromVertexId = letohu32(*((uint32_t*) (source + FDB_EDGE_FROMID_OFFSET)));
    edge->toVertexId = letohu32(*((uint32_t*) (source + FDB_EDGE_TOID_OFFSET)));
    edge->fromNextEdgeId = letohu32(*((uint32_t*) (source + FDB_EDGE_FROMNEXTEDGEID_OFFSET)));
    edge->toNextEdgeId = letohu32(*((uint32_t*) (source + FDB_EDGE_TONEXTEDGEID_OFFSET)));

    fdb_property_load(&edge->value, source + FDB_EDGE_VALUE_OFFSET);
}

void fdb_edge_unload(Edge* edge, uint8_t* dest) {
    uint32_t symbolId = htoleu32(edge->symbolId);
    uint32_t fromVertexId = htoleu32(edge->fromVertexId);
    uint32_t toVertexId = htoleu32(edge->toVertexId);
    uint32_t fromNextEdgeId = htoleu32(edge->fromNextEdgeId);
    uint32_t toNextEdgeId = htoleu32(edge->toNextEdgeId);

    memcpy(dest + FDB_EDGE_SYMBOLID_OFFSET, &symbolId, 4);
    memcpy(dest + FDB_EDGE_FROMID_OFFSET, &fromVertexId, 4);
    memcpy(dest + FDB_EDGE_TOID_OFFSET, &toVertexId, 4);
    memcpy(dest + FDB_EDGE_FROMNEXTEDGEID_OFFSET, &fromNextEdgeId, 4);
    memcpy(dest + FDB_EDGE_TONEXTEDGEID_OFFSET, &toNextEdgeId, 4);

    fdb_property_unload(&edge->value, dest + FDB_EDGE_VALUE_OFFSET);
}

#ifdef FABRICDB_TESTING
#include "../test/test_edge.c"
#endif
