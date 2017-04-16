/*****************************************************************
 * FabricDB Library Edge Header
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
 *     Defines the Edge datatype and operations on it.
 *
 ******************************************************************/

#ifndef __FABRICDB_EDGE_H
#define __FABRICDB_EDGE_H

#include <stdint.h>

#include "property.h"

/******************************************************
 * EDGE FORMAT
 *
 * +-----+------+------------------------------------
 * | pos | size | description
 * +-----+------+------------------------------------
 * |   0 |    4 | symbolId (0 = NULL)
 * |   4 |    9 | value (embedded property)
 * |  13 |    4 | fromVertexId
 * |  17 |    4 | toVertexId
 * |  21 |    4 | fromNextEdgeId
 * |  25 |    4 | toNextEdgeId
 * +-----+------+------------------------------------
 *
 ******************************************************/

#define FDB_EDGE_SYMBOLID_OFFSET 0
#define FDB_EDGE_VALUE_OFFSET 4
#define FDB_EDGE_FROMID_OFFSET 13
#define FDB_EDGE_TOID_OFFSET 17
#define FDB_EDGE_FROMNEXTEDGEID_OFFSET 21
#define FDB_EDGE_TONEXTEDGEID_OFFSET 25
#define FDB_EDGE_DISKSIZE 29

typedef struct Edge {
    uint32_t id;             /* The id of the edge */
    uint32_t symbolId;       /* Reference to a Symbol object */
    Property value;          /* The value of the edge */
    uint32_t fromVertexId;   /* Reference to the from Vertex object */
    uint32_t toVertexId;     /* Reference to the in Vertex object */
    uint32_t fromNextEdgeId; /* Reference to the from vertex's next Edge object */
    uint32_t toNextEdgeId;   /* Reference to the in vertex's next Edge object */
} Edge;

void fdb_edge_load(Edge* edge, uint32_t id, uint8_t* source);
void fdb_edge_unload(Edge* edge, uint8_t* dest);

#endif /* __FABRICDB_EDGE_H */
