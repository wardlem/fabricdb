/*****************************************************************
 * FabricDB Library u32array Implementation
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Generated: 2016-07-04
 * Author: Mark Wardle
 *
 ******************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "u32array.h"
#include "fabric.h"
#include "mem.h"

int u32array_set_size(u32array* arr, uint32_t size) {
    int copyLen;
    uint32_t* newData;

    newData = NULL;
    if (size > 0) {
        newData = fdbmalloczero(sizeof(uint32_t) * size);
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

        memcpy(newData, arr->data, copyLen * sizeof(uint32_t));
        fdbfree(arr->data);
    }

    arr->size = size;
    arr->data = newData;

    return FABRICDB_OK;
}

void u32array_deinit(u32array* arr) {
    fdbfree(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->count = 0;
}

int u32array_reinit(u32array* arr, uint32_t size) {
    u32array_deinit(arr);
    return u32array_set_size(arr, size);
}

int u32array_has(u32array* arr, uint32_t index) {
    return index < arr->count;
}

uint32_t u32array_get_or(u32array* arr, uint32_t index, uint32_t def) {
    if (index >= arr->count) {
        return def;
    }

    return arr->data[index];
}

uint32_t* u32array_get_ref(u32array* arr, uint32_t index) {
    if (index >= arr->count) {
        return NULL;
    }

    return &arr->data[index];
}

int u32array_set(u32array* arr, uint32_t index, uint32_t value) {

    if (index > arr->count) {
        return FABRICDB_EINDEX_OUT_OF_BOUNDS;
    }

    if (index == arr->count) {
        if (arr->size == arr->count) {
            u32array_set_size(arr, arr->size * 2 + 1);
        }

        arr->count++;
    }

    arr->data[index] = value;

    return FABRICDB_OK;
}

int u32array_push(u32array* arr, uint32_t value) {
    return u32array_set(arr, arr->count, value);
}

uint32_t u32array_pop_or(u32array* arr, uint32_t def) {
    uint32_t result = u32array_get_or(arr, arr->count-1, def);
    if (arr->count > 0) {
        arr->count--;
    }
    return result;
}

#ifdef FABRICDB_TESTING
#include "../test/test_u32array.c"
#endif

