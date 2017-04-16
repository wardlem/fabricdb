#include "test_common.h"

void test_vertex_load() {
    Vertex vert;
    Vertex* vertPtr = &vert;

    uint8_t buffer[FDB_VERTEX_DISKSIZE];

    uint32_t symbolId = htoleu32(45);
    uint32_t firstOutEdgeId = htoleu32(981);
    uint32_t firstInEdgeId = htoleu32(3902);
    uint8_t dataType = DATATYPE_INTEGER;
    int64_t dataValue = htolei64(1234567);

    memcpy(buffer + FDB_VERTEX_SYMBOLID_OFFSET, &symbolId, 4);
    memcpy(buffer + FDB_VERTEX_VALUE_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, &dataType, 1);
    memcpy(buffer + FDB_VERTEX_VALUE_OFFSET + FDB_PROPERTY_DATA_OFFSET, &dataValue, 8);
    memcpy(buffer + FDB_VERTEX_FIRSTOUTEDGEID_OFFSET, &firstOutEdgeId, 4);
    memcpy(buffer + FDB_VERTEX_FIRSTINEDGEID_OFFSET, &firstInEdgeId, 4);

    fdb_vertex_load(vertPtr, 435, buffer);

    fdb_assert("Vertex id not set on load", vert.id == 435);
    fdb_assert("Vertex symbol id not set correctly", vert.symbolId == 45);
    fdb_assert("Vertex value type not set correctly", vert.value.dataType == DATATYPE_INTEGER);
    fdb_assert("Vertex value not set correctly", fdb_property_toi64(&vert.value) == 1234567);
    fdb_assert("Vertex first out edge id not set correctly", vert.firstOutEdgeId == 981);
    fdb_assert("Vertex first in edge id not set correctly", vert.firstInEdgeId == 3902);

    fdb_passed;
}

void test_vertex_unload() {
    Vertex vert;
    Vertex* vertPtr = &vert;

    uint8_t buffer[FDB_VERTEX_DISKSIZE];

    uint32_t symbolId = 0;
    uint32_t firstOutEdgeId = 0;
    uint32_t firstInEdgeId = 0;
    uint8_t dataType = 0;
    int64_t dataValue = 0;

    int64_t setValue = htolei64(123456);

    vert.id = 6547;
    vert.symbolId = 145;
    vert.value.dataType = DATATYPE_INTEGER;
    memcpy(vert.value.data, &setValue, 8);
    vert.firstOutEdgeId = 84939;
    vert.firstInEdgeId = 3234;

    fdb_vertex_unload(vertPtr, buffer);

    memcpy(&symbolId, buffer + FDB_VERTEX_SYMBOLID_OFFSET, 4);
    memcpy(&dataType, buffer + FDB_VERTEX_VALUE_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, 1);
    memcpy(&dataValue, buffer + FDB_VERTEX_VALUE_OFFSET + FDB_PROPERTY_DATA_OFFSET, 8);
    memcpy(&firstOutEdgeId, buffer + FDB_VERTEX_FIRSTOUTEDGEID_OFFSET, 4);
    memcpy(&firstInEdgeId, buffer + FDB_VERTEX_FIRSTINEDGEID_OFFSET, 4);

    fdb_assert("Vertex did not store symbol id correctly", letohu32(symbolId) == 145);
    fdb_assert("Vertex did not store data type correctly", dataType == DATATYPE_INTEGER);
    fdb_assert("Vertex did not store data  correctly", letohi64(dataValue) == 123456);
    fdb_assert("Vertex did not store first out edge id correctly", letohu32(firstOutEdgeId) == 84939);
    fdb_assert("Vertex did not store first in edge id correctly", letohu32(firstInEdgeId) == 3234);

    fdb_passed;
}

void test_vertex() {
    fdb_runtest("vertex load", test_vertex_load);
    fdb_runtest("vertex unload", test_vertex_unload);
}
