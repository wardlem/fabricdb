#include "test_common.h"

void test_fstring_load() {
    FString str;
    FString* strPtr = &str;
    uint8_t buffer[64];

    char* text = "Cats and dogs, living together, mass hysteria!";
    uint32_t size = strlen(text);
    uint32_t sizele = htoleu32(size);

    memcpy(buffer, &sizele, 4);
    memcpy(buffer + FDB_FSTRING_DATA_OFFSET, text, size);

    fdb_fstring_load(strPtr, 2, buffer);

    fdb_assert("Length is not correct", str.size == size);
    fdb_assert("Value is not correct", memcmp(text, str.data, size) == 0);
    fdb_assert("Id is not correct", str.id = 2);

    fdb_passed;
}

void test_fstring_unload() {
    FString str;
    FString* strPtr = &str;
    uint8_t buffer[64];

    char* text = "Cats and dogs, living together, mass hysteria!";
    uint32_t size = strlen(text);
    uint32_t sizele = 0;

    str.id = 2;
    str.size = size;
    str.data = (uint8_t*) text;

    fdb_fstring_unload(strPtr, buffer);

    memcpy(&sizele, buffer, 4);

    fdb_assert("Size not stored correctly", letohu32(sizele) == size);
    fdb_assert("Data not stored correctly", memcmp(buffer + FDB_FSTRING_DATA_OFFSET, str.data, size) == 0);

    fdb_passed;
}

void test_fstring_tocstring() {
    FString str;

    char* text = "Cats and dogs, living together, mass hysteria!";
    uint32_t size = strlen(text);
    int error;
    char** out;
    char* result;

    str.id = 2;
    str.size = size;
    str.data = (uint8_t*) text;

    error = fdb_fstring_tocstring(&str, out);


    fdb_assert("Error occurred", error == FABRICDB_OK);
    fdb_assert("Out is null", out != NULL);
    result = *out;
    fdb_assert("Out is null", result != NULL);
    fdb_assert("Returned original data", (void*) result != (void*) text);
    fdb_assert("Wrong length for c string", strlen(result) == size);
    fdb_assert("Not null terminated", result[size + 1] == '\0');
    fdb_assert("Cstring is wrong", memcmp(result, text, size) == 0);

    fdb_assert("Wrong amount of memory allocated", fabricdb_mem_size(result) == size + 1);
    fabricdb_free(result);
    fdb_assert("Did not deallocate", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_fstring() {
    fdb_runtest("fstring load", test_fstring_load);
    fdb_runtest("fstring unload", test_fstring_unload);
    fdb_runtest("fstring to cstring", test_fstring_tocstring);
}
