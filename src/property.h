/*****************************************************************
 * FabricDB Library Property Header
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: July 8, 2016
 * Modified: November 13, 2016
 * Author: Mark Wardle
 * Description:
 *     Defines the Property and LabeledProperty datatypes and operations on them.
 *
 ******************************************************************/

#ifndef __FABRICDB_PROPERTY_H
#define __FABRICDB_PROPERTY_H

#include <stdint.h>
#include "symbol.h"

#define DATATYPE_VOID     0x00
#define DATATYPE_FALSE    0x01
#define DATATYPE_TRUE     0x02
#define DATATYPE_INTEGER  0x03
#define DATATYPE_REAL     0x04
#define DATATYPE_RATIO    0x05
#define DATATYPE_UCHAR    0x06
#define DATATYPE_DATE     0x0F

#define DATATYPE_STRING_0 0x10
#define DATATYPE_STRING_1 0x11
#define DATATYPE_STRING_2 0x12
#define DATATYPE_STRING_3 0x13
#define DATATYPE_STRING_4 0x14
#define DATATYPE_STRING_5 0x15
#define DATATYPE_STRING_6 0x16
#define DATATYPE_STRING_7 0x17
#define DATATYPE_STRING_8 0x18
#define DATATYPE_STRING   0x19
#define DATATYPE_BLOB     0x1F

#define DATATYPE_DOCUMENT 0x20
#define DATATYPE_ARRAY    0x21
#define DATATYPE_SYMBOL   0x22


/******************************************************
 * PROPERTY FORMAT
 *
 * +-----+------+------------------------------------
 * | pos | size | description
 * +-----+------+------------------------------------
 * |   0 |    1 | data type code
 * |   1 |    8 | data
 * +-----+------+------------------------------------
 *
 ******************************************************/
#define FDB_PROPERTY_DATATYPE_OFFSET 0
#define FDB_PROPERTY_DATA_OFFSET 1
#define FDB_PROPERTY_DISKSIZE 9

typedef struct Property {
    uint8_t dataType; /* A one byte data type indicator */
    uint8_t data[8];  /* The data for the property as represented in db file */
    void* dataRef;    /* Reference an in memory string/blob/document/list/symbol object */
} Property;



/******************************************************
 * LABELED PROPERTY FORMAT
 *
 * +-----+------+------------------------------------
 * | pos | size | description
 * +-----+------+------------------------------------
 * |   0 |    4 | label id - a reference to a symbol object
 * |   4 |    9 | property
 * +-----+------+------------------------------------
 *
 ******************************************************/
#define FDB_LABELED_PROPERTY_LABELID_OFFSET 0
#define FDB_LABELED_PROPERTY_PROPERTY_OFFSET 4
#define FDB_LABELED_PROPERTY_DISKSIZE 13

typedef struct LabeledProperty {
    uint32_t labelId;    /* The symbol id for the property's label */
    Property prop;      /* The property */
    Symbol* labelRef;   /* A reference to the label */
} LabeledProperty;

typedef struct ratio {
    int32_t numer;
    int32_t denom;
} ratio;

void fdb_property_load(Property* prop, uint8_t* source);
void fdb_property_unload(Property* prop, uint8_t* dest);
uint8_t fdb_property_tobool(Property* prop);
int64_t fdb_property_toi64(Property* prop);
uint64_t fdb_property_tou64(Property* prop);
int32_t fdb_property_toi32(Property* prop);
uint32_t fdb_property_tou32(Property* prop);
ratio fdb_property_toratio(Property* prop);
double fdb_property_tof64(Property* prop);
void fdb_labeledproperty_load(LabeledProperty* prop, uint8_t* source);
void fdb_labeledproperty_unload(LabeledProperty* prop, uint8_t* dest);

#define fdb_property_isvoid(p) (p->dataType == DATATYPE_VOID)
#define fdb_property_isboolean(p) (p->dataType == DATATYPE_TRUE || p->dataType == DATATYPE_FALSE)
#define fdb_property_isnumeric(p) (p->dataType >= DATATYPE_INTEGER && p->dataType <= DATATYPE_DATE)
#define fdb_property_isstring(p) (p->dataType >= DATATYPE_STRING_0 && p->dataType <= DATATYPE_STRING)
#define fdb_property_isreference(p) (p->dataType >= DATATYPE_STRING)

#endif /* __FABRICDB_PROPERTY_H */
