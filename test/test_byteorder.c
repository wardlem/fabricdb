#include "test_common.h"

void test_memrev16() {
    uint16_t t = 0x0102;
    memrev16(&t);
    fdb_assert("Reverse 16 Failed", t == 0x0201);

    fdb_passed;
}

void test_memrev32() {
    uint32_t t = 0x01020304;
    memrev32(&t);
    fdb_assert("Reverse 32 failed", t == 0x04030201);

    fdb_passed;
}

void test_memrev64() {
    uint64_t t = 0x0102030405060708;
    memrev64(&t);
    fdb_assert("Reverse 64 failed", t == 0x0807060504030201);

    fdb_passed;
}

void test_intrev16() {
    uint16_t t = 0x0102;
    fdb_assert("Reverse 16 failed", intrev16(t) == 0x0201);

    fdb_passed;
}

void test_intrev32() {
    uint32_t t = 0x01020304;
    fdb_assert("Reverse 32 failed", intrev32(t) == 0x04030201);

    fdb_passed;
}

void test_intrev64() {
    uint64_t t = 0x0102030405060708;
    fdb_assert("Reverse 64 failed", intrev64(t) == 0x0807060504030201);

    fdb_passed;
}

void test_byteorder() {
    fdb_runtest("memrev16", test_memrev16);
    fdb_runtest("memrev32", test_memrev32);
    fdb_runtest("memrev64", test_memrev64);
    fdb_runtest("intrev16", test_intrev16);
    fdb_runtest("intrev32", test_intrev32);
    fdb_runtest("intrev64", test_intrev64);

}
