#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ubytearray.h"
#include "fabric.h"
#include "mem.h"

int ubytearray_set_size(ubytearray* arr, uint32_t size) {
    int copyLen;
    uint8_t* newData;

    newData = fdbmalloczero(sizeof(ubytearray) * size);
    if (newData == NULL) {
        return FABRICDB_ENOMEM;
    }

    if (arr->data != NULL) {
        copyLen = arr->count;
        if (size < copyLen){
            arr->count = size;
            copyLen = size;
        }

        memcpy(newData, arr->data, copyLen);
        fdbfree(arr->data);
    }

    arr->size = size;
    arr->data = newData;

    return FABRICDB_OK;
}

void ubytearray_deinit(ubytearray* arr) {
    fdbfree(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->count = 0;
}

int ubytearray_reinit(ubytearray* arr, uint32_t size) {
    ubytearray_deinit(arr);
    return ubytearray_set_size(arr, size);
}

int ubytearray_has(ubytearray* arr, uint32_t index) {
    return index < arr->count;
}

uint8_t ubytearray_get_or(ubytearray* arr, uint32_t index, uint8_t def) {
    if (index >= arr->count) {
        return def;
    }

    return arr->data[index];
}

uint8_t* ubytearray_get_ref(ubytearray* arr, uint32_t index) {
    if (index >= arr->count) {
        return NULL;
    }

    return &arr->data[index];
}

int ubytearray_set(ubytearray* arr, uint32_t index, uint8_t value) {

    if (index > arr->count) {
        return FABRICDB_EINDEX_OUT_OF_BOUNDS;
    }

    if (index == arr->count) {
        if (arr->size == arr->count) {
            ubytearray_set_size(arr, arr->size * 2 + 1);
        }

        arr->count++;
    }

    arr->data[index] = value;

    return FABRICDB_OK;
}

int ubytearray_push(ubytearray* arr, uint8_t value) {
    return ubytearray_set(arr, arr->count, value);
}

uint8_t ubytearray_pop_or(ubytearray* arr, uint8_t def) {
    uint8_t result = ubytearray_get_or(arr, arr->count-1, def);
    arr->count--;
    return result;
}

