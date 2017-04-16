/*****************************************************************
 * FabricDB Library Vertex Header
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
 *     Defines the Vertex datatype and operations on it.
 *
 ******************************************************************/

#ifndef __FABRICDB_VERTEX_H
#define __FABRICDB_VERTEX_H

#include <stdint.h>

#include "property.h"

/******************************************************
 * VERTEX FORMAT
 *
 * +-----+------+------------------------------------
 * | pos | size | description
 * +-----+------+------------------------------------
 * |   0 |    4 | symbolId (0 = NULL)
 * |   4 |    9 | value (embedded property)
 * |  13 |    4 | firstOutEdgeId
 * |  17 |    4 | firstInEdgeId
 * +-----+------+------------------------------------
 *
 ******************************************************/

#define FDB_VERTEX_SYMBOLID_OFFSET 0
#define FDB_VERTEX_VALUE_OFFSET 4
#define FDB_VERTEX_FIRSTOUTEDGEID_OFFSET 13
#define FDB_VERTEX_FIRSTINEDGEID_OFFSET 17
#define FDB_VERTEX_DISKSIZE 21

typedef struct Vertex {
    uint32_t id;              /* The id of the vertex */
    uint32_t symbolId;        /* Reference to a Symbol object */
    Property value;           /* The value of the vertex */
    uint32_t firstOutEdgeId;  /* Reference to an Edge object */
    uint32_t firstInEdgeId;   /* Reference to an Edge object */
} Vertex;

void fdb_vertex_load(Vertex* vert, uint32_t id, uint8_t* source);
void fdb_vertex_unload(Vertex* vert, uint8_t* dest);

#endif /* __FABRICDB_VERTEX_H */
