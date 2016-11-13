#!/usr/bin/env ruby

require 'date'

N = ARGV[-2]
T = ARGV[-1]

cfilename = "./src/#{N}.c"
hfilename = "./src/#{N}.h"
tfilename = "./test/test_#{N}.c"

c_template = %Q~/*****************************************************************
 * FabricDB Library #{N} Implementation
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Generated: #{Date::today()}
 * Author: Mark Wardle
 *
 ******************************************************************/
\#include <stdint.h>
\#include <stdlib.h>
\#include <string.h>
\#include "#{N}.h"
\#include "fabric.h"
\#include "mem.h"

int #{N}_set_size(#{N}* arr, uint32_t size) {
    int copyLen;
    #{T}* newData;

    newData = NULL;
    if (size > 0) {
        newData = fdbmalloczero(sizeof(#{T}) * size);
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

        memcpy(newData, arr->data, copyLen * sizeof(#{T}));
        fdbfree(arr->data);
    }

    arr->size = size;
    arr->data = newData;

    return FABRICDB_OK;
}

void #{N}_deinit(#{N}* arr) {
    fdbfree(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->count = 0;
}

int #{N}_reinit(#{N}* arr, uint32_t size) {
    #{N}_deinit(arr);
    return #{N}_set_size(arr, size);
}

int #{N}_has(#{N}* arr, uint32_t index) {
    return index < arr->count;
}

#{T} #{N}_get_or(#{N}* arr, uint32_t index, #{T} def) {
    if (index >= arr->count) {
        return def;
    }

    return arr->data[index];
}

#{T}* #{N}_get_ref(#{N}* arr, uint32_t index) {
    if (index >= arr->count) {
        return NULL;
    }

    return &arr->data[index];
}

int #{N}_set(#{N}* arr, uint32_t index, #{T} value) {

    if (index > arr->count) {
        return FABRICDB_EINDEX_OUT_OF_BOUNDS;
    }

    if (index == arr->count) {
        if (arr->size == arr->count) {
            #{N}_set_size(arr, arr->size * 2 + 1);
        }

        arr->count++;
    }

    arr->data[index] = value;

    return FABRICDB_OK;
}

int #{N}_push(#{N}* arr, #{T} value) {
    return #{N}_set(arr, arr->count, value);
}

#{T} #{N}_pop_or(#{N}* arr, #{T} def) {
    #{T} result = #{N}_get_or(arr, arr->count-1, def);
    if (arr->count > 0) {
        arr->count--;
    }
    return result;
}

\#ifdef FABRICDB_TESTING
\#include "../test/test_#{N}.c"
\#endif

~

h_template = %Q~\#ifndef __FABRICDB_#{N}_H
\#define __FABRICDB_#{N}_H

typedef struct #{N} {
    uint32_t size;       /* Max size of cache before resizing */
    uint32_t count;      /* Count of currently cached items */
    #{T}* data;          /* Array of items */
} #{N};

int #{N}_set_size(#{N}* arr, uint32_t size);
void #{N}_deinit(#{N}* arr);
int #{N}_reinit(#{N}* arr, uint32_t size);
int #{N}_has(#{N}* arr, uint32_t index);
#{T} #{N}_get_or(#{N}* arr, uint32_t index, #{T} def);
#{T}* #{N}_get_ref(#{N}* arr, uint32_t index);
int #{N}_set(#{N}* arr, uint32_t index, #{T} value);
int #{N}_push(#{N}* arr, #{T} value);
#{T} #{N}_pop_or(#{N}* arr, #{T} def);


\#endif /* __FABRICDB_#{N}_H */
~

t_template = %Q~/*****************************************************************
 * FabricDB Library #{N} Interface
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Generated: #{Date::today()}
 * Author: Mark Wardle
 *
 ******************************************************************/
\#include "test_common.h"

void test_#{N}_set_size() {
    #{N} arr;

    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Failed to set size", #{N}_set_size(&arr, 1) == FABRICDB_OK);
    fdb_assert("Size not set", arr.size == 1);
    fdb_assert("Count is wrong", arr.count == 0);
    fdb_assert("Invalid ptr size", fabricdb_mem_size(arr.data) == sizeof(#{T}));

    fdb_assert("Failed to set size", #{N}_set_size(&arr, 5) == FABRICDB_OK);
    fdb_assert("Size not set", arr.size == 5);
    fdb_assert("Count is wrong", arr.count == 0);
    fdb_assert("Invalid ptr size", fabricdb_mem_size(arr.data) == sizeof(#{T}) * 5);

    fdb_assert("Could not push value", #{N}_push(&arr, 1) == FABRICDB_OK);
    fdb_assert("Could not push value", #{N}_push(&arr, 2) == FABRICDB_OK);
    fdb_assert("Could not push value", #{N}_push(&arr, 4) == FABRICDB_OK);
    fdb_assert("Could not push value", #{N}_push(&arr, 8) == FABRICDB_OK);
    fdb_assert("Could not push value", #{N}_push(&arr, 16) == FABRICDB_OK);

    fdb_assert("Size grew", arr.size == 5);
    fdb_assert("Count was not set", arr.count == 5);
    fdb_assert("Ptr size is wrong", fabricdb_mem_size(arr.data) == sizeof(#{T}) * 5);

    fdb_assert("Values were not set", #{N}_get_or(&arr, 0, 0) == 1);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 1, 0) == 2);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 2, 0) == 4);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 3, 0) == 8);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 4, 0) == 16);

    fdb_assert("Could not push value", #{N}_push(&arr, 0) == FABRICDB_OK);

    fdb_assert("Size did not grow", arr.size > 5);
    fdb_assert("Count was not set", arr.count == 6);
    fdb_assert("Ptr size is wrong", fabricdb_mem_size(arr.data) > sizeof(#{T}) * 5);

    fdb_assert("Values were not set", #{N}_get_or(&arr, 0, 0) == 1);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 1, 0) == 2);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 2, 0) == 4);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 3, 0) == 8);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 4, 0) == 16);
    fdb_assert("Values were not set", #{N}_get_or(&arr, 5, 1) == 0);


    fdb_assert("Failed to set size", #{N}_set_size(&arr, 3) == FABRICDB_OK);
    fdb_assert("Size not set", arr.size == 3);
    fdb_assert("Count is wrong", arr.count == 3);
    fdb_assert("Ptr size is wrong", fabricdb_mem_size(arr.data) == sizeof(#{T}) * 3);

    fdb_assert("Values were not copied", #{N}_get_or(&arr, 0, 0) == 1);
    fdb_assert("Values were not copied", #{N}_get_or(&arr, 1, 0) == 2);
    fdb_assert("Values were not copied", #{N}_get_or(&arr, 2, 0) == 4);
    fdb_assert("Out of bounds returned old value", #{N}_get_or(&arr, 3, 0) == 0);

    #{N}_deinit(&arr);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_#{N}_has() {
    #{N} arr;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Size not initialized to 0", arr.size == 0);
    fdb_assert("Count not initialized to 0", arr.count == 0);
    fdb_assert("Data not initialized to null", arr.data == NULL);

    fdb_assert("Has zero but shouldn't", #{N}_has(&arr, 0) == 0);

    fdb_assert("Could not add a value", #{N}_set(&arr, 0, 100) == FABRICDB_OK);
    fdb_assert("Could not add a value", #{N}_set(&arr, 1, 101) == FABRICDB_OK);
    fdb_assert("Could not add a value", #{N}_set(&arr, 2, 102) == FABRICDB_OK);

    fdb_assert("Does not have 0 index", #{N}_has(&arr, 0) == 1);
    fdb_assert("Does not have 1 index", #{N}_has(&arr, 1) == 1);
    fdb_assert("Does not have 2 index", #{N}_has(&arr, 2) == 1);
    fdb_assert("Has 3 index, but should not", #{N}_has(&arr, 3) == 0);

    #{N}_deinit(&arr);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_#{N}_get_or() {
    #{N} arr;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Size not initialized to 0", arr.size == 0);
    fdb_assert("Count not initialized to 0", arr.count == 0);
    fdb_assert("Data not initialized to null", arr.data == NULL);

    fdb_assert("Has zero but shouldn't", #{N}_get_or(&arr, 0, 1) == 1);

    fdb_assert("Could not add a value", #{N}_set(&arr, 0, 100) == FABRICDB_OK);
    fdb_assert("Could not add a value", #{N}_set(&arr, 1, 101) == FABRICDB_OK);
    fdb_assert("Could not add a value", #{N}_set(&arr, 2, 102) == FABRICDB_OK);

    fdb_assert("Does not have 0 index", #{N}_get_or(&arr, 0, 12) == 100);
    fdb_assert("Does not have 1 index", #{N}_get_or(&arr, 1, 12) == 101);
    fdb_assert("Does not have 2 index", #{N}_get_or(&arr, 2, 12) == 102);
    fdb_assert("Has 3 index, but should not", #{N}_get_or(&arr, 3, 12) == 12);

    #{N}_deinit(&arr);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_#{N}_get_ref() {
    #{N} arr;
    #{T}* v;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Size not initialized to 0", arr.size == 0);
    fdb_assert("Count not initialized to 0", arr.count == 0);
    fdb_assert("Data not initialized to null", arr.data == NULL);

    fdb_assert("Has zero but shouldn't", #{N}_get_ref(&arr, 0) == NULL);

    fdb_assert("Could not add a value", #{N}_set(&arr, 0, 100) == FABRICDB_OK);
    fdb_assert("Could not add a value", #{N}_set(&arr, 1, 101) == FABRICDB_OK);
    fdb_assert("Could not add a value", #{N}_set(&arr, 2, 102) == FABRICDB_OK);

    v = #{N}_get_ref(&arr, 0);
    fdb_assert("Does not have 0 index", *v == 100);
    v = #{N}_get_ref(&arr, 1);
    fdb_assert("Does not have 1 index", *v == 101);
    v = #{N}_get_ref(&arr, 2);
    fdb_assert("Does not have 2 index", *v == 102);
    *v = 13;
    fdb_assert("Does not have 2 index", #{N}_get_or(&arr, 2, 0) == 13);
    v = #{N}_get_ref(&arr, 3);
    fdb_assert("Has 3 index, but should not", v == NULL);

    #{N}_deinit(&arr);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_#{N}_pop_or() {
    #{N} arr;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Could not push value", #{N}_push(&arr, 1) == FABRICDB_OK);
    fdb_assert("Could not push value", #{N}_push(&arr, 2) == FABRICDB_OK);
    fdb_assert("Could not push value", #{N}_push(&arr, 4) == FABRICDB_OK);
    fdb_assert("Could not push value", #{N}_push(&arr, 8) == FABRICDB_OK);
    fdb_assert("Could not push value", #{N}_push(&arr, 16) == FABRICDB_OK);

    fdb_assert("Count was not set", arr.count == 5);

    fdb_assert("Did not pop value", #{N}_pop_or(&arr, 0) == 16);
    fdb_assert("Count not updated", arr.count == 4);
    fdb_assert("Did not pop value", #{N}_pop_or(&arr, 0) == 8);
    fdb_assert("Count not updated", arr.count == 3);
    fdb_assert("Did not pop value", #{N}_pop_or(&arr, 0) == 4);
    fdb_assert("Count not updated", arr.count == 2);
    fdb_assert("Did not pop value", #{N}_pop_or(&arr, 0) == 2);
    fdb_assert("Count not updated", arr.count == 1);
    fdb_assert("Did not pop value", #{N}_pop_or(&arr, 0) == 1);
    fdb_assert("Count not updated", arr.count == 0);
    fdb_assert("Did not pop value", #{N}_pop_or(&arr, 0) == 0);
    fdb_assert("Count not updated", arr.count == 0);

    #{N}_deinit(&arr);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;

}

void test_#{N}() {
    fdb_runtest("#{N} set size", test_#{N}_set_size);
    fdb_runtest("#{N} has", test_#{N}_has);
    fdb_runtest("#{N} get or", test_#{N}_get_or);
    fdb_runtest("#{N} get ref", test_#{N}_get_ref);
    fdb_runtest("#{N} pop or", test_#{N}_pop_or);
}
~

File.open(cfilename, "w") do |f|
    f.write(c_template)
end

File.open(hfilename, "w") do |f|
    f.write(h_template)
end

File.open(tfilename, "w") do |f|
    f.write(t_template)
end

puts "Generated #{cfilename} and #{hfilename}"
