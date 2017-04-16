#include "test_common.h"

void test_document_load() {
    Document doc;
    Document* docPtr = &doc;

    uint8_t buffer[FDB_DOCUMENT_DISKSIZE];

    uint8_t dataType = DATATYPE_INTEGER;
    uint32_t labelId = htoleu32(35135);
    int64_t dataValue = htolei64(823824);
    uint64_t nextEntryId = htoleu64(82349);

    memcpy(buffer + FDB_DOCUMENT_ENTRY_OFFSET + FDB_LABELED_PROPERTY_LABELID_OFFSET, &labelId, 4);
    memcpy(buffer + FDB_DOCUMENT_ENTRY_OFFSET + FDB_LABELED_PROPERTY_PROPERTY_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, &dataType, 1);
    memcpy(buffer + FDB_DOCUMENT_ENTRY_OFFSET + FDB_LABELED_PROPERTY_PROPERTY_OFFSET + FDB_PROPERTY_DATA_OFFSET, &dataValue, 8);
    memcpy(buffer + FDB_DOCUMENT_NEXTENTRYID_OFFSET, &nextEntryId, 8);

    fdb_document_load(docPtr, 3254, buffer);

    fdb_assert("Document id not set on load", doc.id == 3254);
    fdb_assert("Document entry label id not set correctly", doc.entry.labelId == 35135);
    fdb_assert("Document value type not set correctly", doc.entry.prop.dataType == DATATYPE_INTEGER);
    fdb_assert("Document value not set correctly", fdb_property_toi64(&doc.entry.prop) == 823824);
    fdb_assert("Document next entry id", doc.nextEntryId == 82349);

    fdb_passed;
}

void test_document_unload() {
    Document doc;
    Document* docPtr = &doc;

    uint8_t buffer[FDB_DOCUMENT_DISKSIZE];

    uint32_t labelId = 0;
    uint8_t dataType = 0;
    int64_t dataValue = 0;
    uint64_t nextEntryId = 0;

    int64_t setValue = htolei64(543465432);

    doc.id = 5436;
    doc.entry.labelId = 12312451;
    doc.entry.prop.dataType = DATATYPE_INTEGER;
    memcpy(doc.entry.prop.data, &setValue, 8);
    doc.nextEntryId = 2124321;

    fdb_document_unload(docPtr, buffer);

    memcpy(&labelId, buffer + FDB_DOCUMENT_ENTRY_OFFSET + FDB_LABELED_PROPERTY_LABELID_OFFSET, 4);
    memcpy(&dataType, buffer + FDB_DOCUMENT_ENTRY_OFFSET + FDB_LABELED_PROPERTY_PROPERTY_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, 1);
    memcpy(&dataValue, buffer + FDB_DOCUMENT_ENTRY_OFFSET + FDB_LABELED_PROPERTY_PROPERTY_OFFSET + FDB_PROPERTY_DATA_OFFSET, 8);
    memcpy(&nextEntryId, buffer + FDB_DOCUMENT_NEXTENTRYID_OFFSET, 8);

    fdb_assert("Document did not store label id correctly", labelId == 12312451);
    fdb_assert("Document did not store data type correctly", dataType == DATATYPE_INTEGER);
    fdb_assert("Document did not store data correctly", letohi64(dataValue) == 543465432);
    fdb_assert("Document did not store next entry id correctly", letohu64(nextEntryId) == 2124321);

    fdb_passed;
}

void test_document() {
    fdb_runtest("document load", test_document_load);
    fdb_runtest("document unload", test_document_unload);
}
