#!/usr/bin/env ruby

N = ARGV[-2]
T = ARGV[-1]

cfilename = "./src/#{N}.c"
hfilename = "./src/#{N}.h"

c_template = %Q~\#include <stdint.h>
\#include <stdlib.h>
\#include <string.h>
\#include "#{N}.h"
\#include "fabric.h"
\#include "mem.h"

int #{N}_set_size(#{N}* arr, uint32_t size) {
    int copyLen;
    #{T}* newData;

    newData = fdbmalloczero(sizeof(#{N}) * size);
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
    arr->count--;
    return result;
}

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

File.open(cfilename, "w") do |f|
    f.write(c_template)
end

File.open(hfilename, "w") do |f|
    f.write(h_template)
end

puts "Generated #{cfilename} and #{hfilename}"
