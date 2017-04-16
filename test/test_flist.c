#include "test_common.h"

void test_flist_load() {
    FList list;
    FList* listPtr = &list;

    uint8_t buffer[FDB_FLIST_DISKSIZE];

    uint8_t dataType = DATATYPE_INTEGER;
    int64_t dataValue = htolei64(823824);
    uint64_t nextEntryId = htoleu64(82349);

    memcpy(buffer + FDB_FLIST_ENTRY_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, &dataType, 1);
    memcpy(buffer + FDB_FLIST_ENTRY_OFFSET + FDB_PROPERTY_DATA_OFFSET, &dataValue, 8);
    memcpy(buffer + FDB_FLIST_NEXTENTRYID_OFFSET, &nextEntryId, 8);

    fdb_flist_load(listPtr, 3254, buffer);

    fdb_assert("FList id not set on load", list.id == 3254);
    fdb_assert("FList value type not set correctly", list.entry.dataType == DATATYPE_INTEGER);
    fdb_assert("FList value not set correctly", fdb_property_toi64(&list.entry) == 823824);
    fdb_assert("FList next entry id", list.nextEntryId == 82349);

    fdb_passed;
}

void test_flist_unload() {
    FList list;
    FList* listPtr = &list;

    uint8_t buffer[FDB_FLIST_DISKSIZE];

    uint8_t dataType = 0;
    int64_t dataValue = 0;
    uint64_t nextEntryId = 0;

    int64_t setValue = htolei64(543465432);

    list.id = 5436;
    list.entry.dataType = DATATYPE_INTEGER;
    memcpy(list.entry.data, &setValue, 8);
    list.nextEntryId = 2124321;

    fdb_flist_unload(listPtr, buffer);

    memcpy(&dataType, buffer + FDB_FLIST_ENTRY_OFFSET + FDB_PROPERTY_DATATYPE_OFFSET, 1);
    memcpy(&dataValue, buffer + FDB_FLIST_ENTRY_OFFSET + FDB_PROPERTY_DATA_OFFSET, 8);
    memcpy(&nextEntryId, buffer + FDB_FLIST_NEXTENTRYID_OFFSET, 8);

    fdb_assert("FList did not store data type correctly", dataType == DATATYPE_INTEGER);
    fdb_assert("FList did not store data correctly", letohi64(dataValue) == 543465432);
    fdb_assert("FList did not store next entry id correctly", letohu64(nextEntryId) == 2124321);

    fdb_passed;
}

void test_flist() {
    fdb_runtest("flist load", test_flist_load);
    fdb_runtest("flist unload", test_flist_unload);
}
