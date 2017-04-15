#include "test_common.h"

void test_vertex_load() {
    Vertex vert;
    Vertex* vertPtr = &vert;

    uint8_t buffer[FDB_VERTEX_DISKSIZE];

    uint16_t symbolId = htoleu16(45);
    uint32_t firstEdgeId = htoleu32(981);
    uint8_t dataType = DATATYPE_INTEGER;
    int64_t dataValue = htolei64(1234567);

    memcpy(buffer + FDB_VERTEX_SYMBOLID_OFFSET, &symbolId, 2);
    memcpy(buffer + FDB_VERTEX_VALUE_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, &dataType, 1);
    memcpy(buffer + FDB_VERTEX_VALUE_OFFSET + FDB_PROPERTY_DATA_OFFSET, &dataValue, 8);
    memcpy(buffer + FDB_VERTEX_FIRSTEDGEID_OFFSET, &firstEdgeId, 4);

    fdb_vertex_load(vertPtr, 435, buffer);

    fdb_assert("Vertex id not set on load", vert.id == 435);
    fdb_assert("Vertex symbol id not set correctly", vert.symbolId == 45);
    fdb_assert("Vertex value type not set correctly", vert.value.dataType == DATATYPE_INTEGER);
    fdb_assert("Vertex value not set correctly", fdb_property_toi64(&vert.value) == 1234567);
    fdb_assert("Vertex first edge id not set correctly", vert.firstEdgeId == 981);

    fdb_passed;
}

void test_vertex_unload() {
    Vertex vert;
    Vertex* vertPtr;

    uint8_t buffer[FDB_VERTEX_DISKSIZE];

    uint32_t symbolId = 0;
    uint32_t firstEdgeId = 0;
    uint8_t dataType = 0;
    int64_t dataValue = 0;

    int64_t setValue = htolei64(123456);

    vert.id = 6547;
    vert.symbolId = 145;
    vert.value.dataType = DATATYPE_INTEGER;
    memcpy(vert.value.data, &setValue, 8);
    vert.firstEdgeId = 84939;

    fdb_vertex_unload(vertPtr, buffer);

    memcpy(&symbolId, buffer + FDB_VERTEX_SYMBOLID_OFFSET, 4);
    memcpy(&dataType, buffer + FDB_VERTEX_VALUE_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, 1);
    memcpy(&dataValue, buffer + FDB_VERTEX_VALUE_OFFSET + FDB_PROPERTY_DATA_OFFSET, 8);
    memcpy(&firstEdgeId, buffer + FDB_VERTEX_FIRSTEDGEID_OFFSET, 4);

    fdb_assert("Vertex did not store symbol id correctly", letohu32(symbolId) == 145);
    fdb_assert("Vertex did not store data type correctly", dataType == DATATYPE_INTEGER);
    fdb_assert("Vertex did not store data  correctly", letohi64(dataValue) == 123456);
    fdb_assert("Vertex did not store first edge id correctly", letohu32(firstEdgeId) == 84939);

    fdb_passed;
}

void test_vertex() {
    fdb_runtest("vertex load", test_vertex_load);
    fdb_runtest("vertex unload", test_vertex_unload);
}
