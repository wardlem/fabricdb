#include "test_common.h"

void test_edge_load() {
    Edge edge;
    Edge* edgePtr = &edge;

    uint8_t buffer[FDB_EDGE_DISKSIZE];

    uint32_t symbolId = htoleu32(234);
    uint32_t fromVertexId = htoleu32(412);
    uint32_t toVertexId = htoleu32(9728);
    uint32_t fromNextEdgeId = htoleu32(328992);
    uint32_t toNextEdgeId = htoleu32(3892873724);
    uint8_t dataType = DATATYPE_INTEGER;
    int64_t dataValue = htolei64(1231821823);

    memcpy(buffer + FDB_EDGE_SYMBOLID_OFFSET, &symbolId, 4);
    memcpy(buffer + FDB_EDGE_VALUE_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, &dataType, 1);
    memcpy(buffer + FDB_EDGE_VALUE_OFFSET + FDB_PROPERTY_DATA_OFFSET, &dataValue, 8);
    memcpy(buffer + FDB_EDGE_FROMID_OFFSET, &fromVertexId, 4);
    memcpy(buffer + FDB_EDGE_TOID_OFFSET, &toVertexId, 4);
    memcpy(buffer + FDB_EDGE_FROMNEXTEDGEID_OFFSET, &fromNextEdgeId, 4);
    memcpy(buffer + FDB_EDGE_TONEXTEDGEID_OFFSET, &toNextEdgeId, 4);

    fdb_edge_load(edgePtr, 4345, buffer);

    fdb_assert("Edge id not set on load", edge.id == 4345);
    fdb_assert("Edge symbol id not set correctly", edge.symbolId == 234);
    fdb_assert("Edge value type not set correctly", edge.value.dataType == DATATYPE_INTEGER);
    fdb_assert("Edge value not set correctly", fdb_property_toi64(&edge.value) == 1231821823);
    fdb_assert("Edge from vertex id not set correctly", edge.fromVertexId == 412);
    fdb_assert("Edge to vertex id not set correctly", edge.toVertexId == 9728);
    fdb_assert("Edge from next edge id not set correctly", edge.fromNextEdgeId == 328992);
    fdb_assert("Edge to next edge id not set correctly", edge.toNextEdgeId == 3892873724);

    fdb_passed;
}

void test_edge_unload() {
    Edge edge;
    Edge* edgePtr = &edge;

    uint8_t buffer[FDB_EDGE_DISKSIZE];

    uint32_t symbolId =  0;
    uint32_t fromVertexId = 0;
    uint32_t toVertexId = 0;
    uint32_t fromNextEdgeId = 0;
    uint32_t toNextEdgeId = 0;
    uint8_t dataType = 0;
    int64_t dataValue = 0;

    int64_t setValue = htolei64(392938273);

    edge.id = 4345;
    edge.symbolId = 2345;
    edge.value.dataType = DATATYPE_INTEGER;
    memcpy(edge.value.data, &setValue, 8);
    edge.fromVertexId = 829342;
    edge.toVertexId = 38297297;
    edge.fromNextEdgeId = 8328;
    edge.toNextEdgeId = 839293870;

    fdb_edge_unload(edgePtr, buffer);

    memcpy(&symbolId, buffer + FDB_EDGE_SYMBOLID_OFFSET, 4);
    memcpy(&fromVertexId, buffer + FDB_EDGE_FROMID_OFFSET, 4);
    memcpy(&toVertexId, buffer + FDB_EDGE_TOID_OFFSET, 4);
    memcpy(&fromNextEdgeId, buffer + FDB_EDGE_FROMNEXTEDGEID_OFFSET, 4);
    memcpy(&toNextEdgeId, buffer + FDB_EDGE_TONEXTEDGEID_OFFSET, 4);
    memcpy(&dataType, buffer + FDB_EDGE_VALUE_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, 1);
    memcpy(&dataValue, buffer + FDB_EDGE_VALUE_OFFSET + FDB_PROPERTY_DATA_OFFSET, 8);

    fdb_assert("Edge did not store symbol id correctly", letohu32(symbolId) == 2345);
    fdb_assert("Edge did not store data type correctly", dataType == DATATYPE_INTEGER);
    fdb_assert("Edge did not store data value correctly", letohi64(dataValue) == 392938273);
    fdb_assert("Edge did not store from vertex id correctly", letohu32(fromVertexId) == 829342);
    fdb_assert("Edge did not store to vertex id correctly", letohu32(toVertexId) == 38297297);
    fdb_assert("Edge did not store from next edge id correctly", letohu32(fromNextEdgeId) == 8328);
    fdb_assert("Edge did not store to next edge id correctly", letohu32(toNextEdgeId) == 839293870);

    fdb_passed;
}

void test_edge() {
    fdb_runtest("edge load", test_edge_load);
    fdb_runtest("edge unload", test_edge_unload);
}
