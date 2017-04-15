#include "test_common.h"

void test_symbol_load() {
    Symbol symbol;
    Symbol* symbolPtr = &symbol;

    uint8_t buffer[FDB_SYMBOL_DISKSIZE];

    uint32_t refCount = htoleu32(24);
    uint64_t stringId = htoleu64(53);

    memcpy(buffer + FDB_SYMBOL_REFCOUNT_OFFSET, &refCount, 4);
    memcpy(buffer + FDB_SYMBOL_STRINGID_OFFSET, &stringId, 8);

    fdb_symbol_load(symbolPtr, 34, buffer);

    fdb_assert("Id not set", symbol.id == 34);
    fdb_assert("Ref count is wrong", symbol.refCount == 24);
    fdb_assert("String id is wrong", symbol.stringId == 53);

    fdb_passed;
}

void test_symbol_unload() {
    Symbol symbol;
    Symbol* symbolPtr = &symbol;

    uint8_t buffer[FDB_SYMBOL_DISKSIZE];

    uint32_t refCount = 0;
    uint64_t stringId = 0;

    symbol.id = 34;
    symbol.refCount = 456;
    symbol.stringId = 35;

    fdb_symbol_unload(symbolPtr, buffer);

    memcpy(&refCount, buffer + FDB_SYMBOL_REFCOUNT_OFFSET, 4);
    memcpy(&stringId, buffer + FDB_SYMBOL_STRINGID_OFFSET, 8);

    fdb_assert("Ref count not unloaded properly", letohu32(refCount) == 456);
    fdb_assert("String id not unloaded properly", letohu64(stringId) == 35);


    fdb_passed;
}

void test_symbol() {
    fdb_runtest("symbol load", test_symbol_load);
    fdb_runtest("symbol unload", test_symbol_unload);
}
