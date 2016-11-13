#!/usr/bin/env ruby

require "date"

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
\#include "#{N}.h"
\#include "fabric.h"
\#include "mem.h"

int #{N}_set_size(#{N}* map, uint32_t size) {
    uint32_t indexOld;
    uint32_t indexNew;
    uint32_t oldSize;
    #{N}_entry** oldItems;
    #{N}_entry** newItems;
    #{N}_entry* current;
    #{N}_entry* next;


    oldItems = map->items;
    newItems = (#{N}_entry**)fdbmalloczero(sizeof(#{N}_entry*) * size);
    if (newItems == NULL) {
        return FABRICDB_ENOMEM;
    }
    oldSize = map->size;


    if (map->items != NULL) {

        indexOld = 0;
        while (indexOld < oldSize) {
            current = oldItems[indexOld];
            while(current != NULL) {
                next = current->next;
                indexNew = current->key % size;
                current->next = newItems[indexNew];
                newItems[indexNew] = current;
                current = next;
            }
            indexOld++;
        }

        fdbfree(oldItems);
    }

    map->items = newItems;
    map->size = size;
    map->resizeRatio = 0.7;
    map->fillRatio = (float)map->count / (float)map->size;

    return FABRICDB_OK;
}

static void #{N}_free_item(#{N}_entry* item) {
    if (item == NULL) {
        return;
    }

    if(item->next != NULL) {
        #{N}_free_item(item->next);
    }

    fdbfree(item);
}

void #{N}_deinit(#{N}* map) {
    #{N}_entry* current;
    uint32_t index;

    if (map->items == NULL) {
        return;
    }

    index = 0;
    while(index < map->size) {
        //current = *(map->items + sizeof(#{N}_entry*) * index);
        current = map->items[index];
        #{N}_free_item(current);
        index++;
    }

    fdbfree(map->items);
    map->items = NULL;
    map->size = 0;
}

int #{N}_reinit(#{N}* map, uint32_t size) {
    #{N}_deinit(map);
    return #{N}_set_size(map, size);
}

int #{N}_has(#{N}* map, uint32_t key) {
    uint32_t index;
    #{N}_entry* current;

    index = key % map->size;
    current = map->items[index];

    while(current != NULL) {
        if(current->key == key) {
            return 1;
        }
        current = current->next;
    }

    return 0;
}

#{T} #{N}_get_or(#{N}* map, uint32_t key, #{T} def) {
    uint32_t index = key % map->size;
    #{N}_entry* current = map->items[index];

    while(current != NULL) {
        if(current->key == key) {
            return current->value;
        }
        current = current->next;
    }

    return def;
}

#{T}* #{N}_get_ref(#{N}* map, uint32_t key) {
    #{T}* out = NULL;
    uint32_t index = key % map->size;
    #{N}_entry* current = map->items[index];

    while(current != NULL) {
        if(current->key == key) {
            out = &current->value;
            break;
        }
        current = current->next;
    }

    return out;
}

int #{N}_set(#{N}* map, uint32_t key, #{T} value) {
    #{N}_entry* entry;
    #{N}_entry* current;
    uint32_t index;
    int rc = FABRICDB_OK;

    entry = fdbmalloc(sizeof(#{N}_entry));
    if (entry == NULL) {
        return FABRICDB_ENOMEM;
    }

    if (map->resizeRatio < map->fillRatio) {
        rc = #{N}_set_size(map, map->size * 2 + 1);
        if (rc != FABRICDB_OK) {
            fdbfree(entry);
            return rc;
        }
    }

    entry->key = key;
    entry->value = value;
    entry->next = NULL;

    index = key % map->size;
    current = map->items[index];

    if (current == NULL) {
        map->items[index] = entry;
        map->count++;
    } else {
        while(current != NULL) {
            if (current->key == key) {
                current->value = value;
                break;
            }
            if (current->next == NULL) {
                current->next = entry;
                map->count++;
                break;
            }
            current = current->next;
        }
    }

    if (map->size == 0) {
        map->fillRatio = 1.0;
    } else {
        map->fillRatio = (float) map->count / (float) map->size;
    }

    return FABRICDB_OK;
}

\#ifdef FABRICDB_TESTING
\#include "../test/test_#{N}.c"
\#endif

~

h_template = %Q~/*****************************************************************
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
\#ifndef __FABRICDB_#{N}_H
\#define __FABRICDB_#{N}_H
typedef struct #{N}_entry {
    uint32_t key;
    #{T} value;
    struct #{N}_entry* next;
} #{N}_entry;

typedef struct #{N} {
    uint32_t size;       /* Max size of map before resizing */
    uint32_t count;      /* Count of currently mapd items */
    float resizeRatio;   /* When to resize */
    float fillRatio;     /* count / size */
    #{N}_entry** items;  /* Array of item buckets */
} #{N};

int #{N}_set_size(#{N}* map, uint32_t size);
void #{N}_deinit(#{N}* map);
int #{N}_reinit(#{N}* map, uint32_t size);
int #{N}_has(#{N}* map, uint32_t key);
#{T} #{N}_get_or(#{N}* map, uint32_t key, #{T} def);
#{T}* #{N}_get_ref(#{N}* map, uint32_t key);
int #{N}_set(#{N}* map, uint32_t key, #{T} value);

\#endif /* __FABRICDB_#{N}_H */
~

t_template = %Q~\#include "test_common.h"
void test_#{N}_set_size() {
    #{N} map;
    int memUsed;
    int testV;

    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);
    fdb_assert("Count is set", map.count == 0);

    fdb_assert("Set size failed", #{N}_set_size(&map, 5) == FABRICDB_OK);
    fdb_assert("Did not allocate memory", fabricdb_mem_used() > 0);
    fdb_assert("Did not set size", map.size == 5);
    fdb_assert("Set count", map.count == 0);

    memUsed = fabricdb_mem_used();

    fdb_assert("Set size failed", #{N}_set_size(&map, 10) == FABRICDB_OK);
    fdb_assert("Did not allocate memory", fabricdb_mem_used() > 0);
    fdb_assert("Old data not freed", fabricdb_mem_used() - (5 * sizeof(#{T})) == memUsed);
    fdb_assert("Did not set size", map.size == 10);
    fdb_assert("Set count", map.count == 0);

    testV = 1+map.size;
    fdb_assert("Insert failed", #{N}_set(&map, 1, (#{T})2) == FABRICDB_OK);
    fdb_assert("Insert failed", #{N}_set(&map, 3, (#{T})8) == FABRICDB_OK);
    fdb_assert("Insert failed", #{N}_set(&map, testV, (#{T})1) == FABRICDB_OK);

    fdb_assert("Does not have value 1", #{N}_has(&map, 1) == 1);
    fdb_assert("Does not have value 3", #{N}_has(&map, 3) == 1);
    fdb_assert("Does not have testV", #{N}_has(&map,testV) == 1);
    fdb_assert("Has value 2", #{N}_has(&map, 2) == 0);
    fdb_assert("Was resized", map.size == 10);
    fdb_assert("Count not set", map.count == 3);

    fdb_assert("Set size failed", #{N}_set_size(&map, 20) == FABRICDB_OK);
    fdb_assert("Size not set", map.size == 20);
    fdb_assert("Count was changed", map.count == 3);

    fdb_assert("Does not have value 1", #{N}_has(&map, 1) == 1);
    fdb_assert("Does not have value 3", #{N}_has(&map, 3) == 1);
    fdb_assert("Does not have value testV", #{N}_has(&map,testV) == 1);
    fdb_assert("Has value 2", #{N}_has(&map, 2) == 0);

    fdb_assert("Set size failed", #{N}_set_size(&map, 3) == FABRICDB_OK);
    fdb_assert("Size not set", map.size == 3);
    fdb_assert("Count was changed", map.count == 3);

    fdb_assert("Does not have value 1", #{N}_has(&map, 1) == 1);
    fdb_assert("Does not have value 3", #{N}_has(&map, 3) == 1);
    fdb_assert("Does not have value testV", #{N}_has(&map,testV) == 1);
    fdb_assert("Has value 2", #{N}_has(&map, 2) == 0);

    /* trigger auto resize */
    fdb_assert("Insert failed", #{N}_set(&map, 5, (#{T})32) == FABRICDB_OK);
    fdb_assert("Size not set", map.size > 3);
    fdb_assert("Count was changed", map.count == 4);

    fdb_assert("Does not have value 1", #{N}_has(&map, 1) == 1);
    fdb_assert("Does not have value 3", #{N}_has(&map, 3) == 1);
    fdb_assert("Does not have value testV", #{N}_has(&map,testV) == 1);
    fdb_assert("Does not have value 5", #{N}_has(&map,5) == 1);
    fdb_assert("Has value 2", #{N}_has(&map, 2) == 0);

    #{N}_deinit(&map);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_#{N}_get_ref() {
    #{N} map;
    #{T}* v;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);
    fdb_assert("Count is set", map.count == 0);

    fdb_assert("Resize failed", #{N}_set_size(&map, 3) == FABRICDB_OK);

    fdb_assert("Insert failed", #{N}_set(&map, 1, (#{T})2) == FABRICDB_OK);
    fdb_assert("Insert failed", #{N}_set(&map, 3, (#{T})8) == FABRICDB_OK);
    fdb_assert("Insert failed", #{N}_set(&map, 9, (#{T})1) == FABRICDB_OK);

    /* force a conflict */
    fdb_assert("Resize failed", #{N}_set_size(&map, 3) == FABRICDB_OK);
    fdb_assert("Resize did not set size", map.size == 3);

    v = #{N}_get_ref(&map, 1);
    fdb_assert("Did not return correct reference", v != NULL);
    fdb_assert("Did not return correct value", *v == (#{T})2);
    v = #{N}_get_ref(&map, 3);
    fdb_assert("Did not return correct reference", v != NULL);
    fdb_assert("Did not return correct value", *v == (#{T})8);
    v = #{N}_get_ref(&map, 9);
    fdb_assert("Did not return correct reference", v != NULL);
    fdb_assert("Did not return correct value", *v == (#{T})1);
    *v = (#{T})3;
    fdb_assert("Did not update reference value", #{N}_get_or(&map, 9, 0) == (#{T})3);
    v = #{N}_get_ref(&map, 2);
    fdb_assert("Did not return null", v == NULL);
    v = #{N}_get_ref(&map, 6);
    fdb_assert("Did not return null", v == NULL);


    #{N}_deinit(&map);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_#{N}() {
    fdb_runtest("#{N} set size", test_#{N}_set_size);
    fdb_runtest("#{N} get ref", test_#{N}_get_ref);
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

puts "Generated #{cfilename} #{hfilename} #{tfilename}"
