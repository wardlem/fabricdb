#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "u8array.h"
#include "fabric.h"
#include "mem.h"

int u8array_set_size(u8array* arr, uint32_t size) {
    int copyLen;
    uint8_t* newData;

    newData = NULL;
    if (size > 0) {
        newData = fdbmalloczero(sizeof(uint8_t) * size);
        if (newData == NULL) {
            return FABRICDB_ENOMEM;
        }
    }

    if (arr->data != NULL && arr->size > 0) {
        copyLen = arr->count;
        if (size < copyLen){
            arr->count = size;
            copyLen = size;
        }

        memcpy(newData, arr->data, copyLen * sizeof(uint8_t));
        fdbfree(arr->data);
    }

    arr->size = size;
    arr->data = newData;

    return FABRICDB_OK;
}

void u8array_deinit(u8array* arr) {
    fdbfree(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->count = 0;
}

int u8array_reinit(u8array* arr, uint32_t size) {
    u8array_deinit(arr);
    return u8array_set_size(arr, size);
}

int u8array_has(u8array* arr, uint32_t index) {
    return index < arr->count;
}

uint8_t u8array_get_or(u8array* arr, uint32_t index, uint8_t def) {
    if (index >= arr->count) {
        return def;
    }

    return arr->data[index];
}

uint8_t* u8array_get_ref(u8array* arr, uint32_t index) {
    if (index >= arr->count) {
        return NULL;
    }

    return &arr->data[index];
}

int u8array_set(u8array* arr, uint32_t index, uint8_t value) {

    if (index > arr->count) {
        return FABRICDB_EINDEX_OUT_OF_BOUNDS;
    }

    if (index == arr->count) {
        if (arr->size == arr->count) {
            u8array_set_size(arr, arr->size * 2 + 1);
        }

        arr->count++;
    }

    arr->data[index] = value;

    return FABRICDB_OK;
}

int u8array_push(u8array* arr, uint8_t value) {
    return u8array_set(arr, arr->count, value);
}

uint8_t u8array_pop_or(u8array* arr, uint8_t def) {
    uint8_t result = u8array_get_or(arr, arr->count-1, def);
    if (arr->count > 0) {
        arr->count--;
    }
    return result;
}

#ifdef FABRICDB_TESTING
#include "../test/test_u8array.c"
#endif

