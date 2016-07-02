#!/usr/bin/env ruby

N = ARGV[-2]
T = ARGV[-1]

cfilename = "./src/#{N}.c"
hfilename = "./src/#{N}.h"

c_template = %Q~\#include <stdint.h>
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
            current = *(oldItems + sizeof(#{N}_entry) * indexOld);
            while(current != NULL) {
                next = current->next;
                indexNew = current-> key % size;
                current->next = *(newItems + sizeof(#{N}_entry*) * indexNew);
                *(newItems + sizeof(#{N}_entry) * indexNew) = current;
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
        current = *(map->items + sizeof(#{N}_entry*) * index);
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
    current = *(map->items + sizeof(#{N}_entry*) * index);

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
    #{N}_entry* current = *(map->items + sizeof(#{N}_entry*) * index);

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
    #{N}_entry* current = *(map->items + sizeof(#{N}_entry*) * index);

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
    current = *(map->items + sizeof(#{N}_entry*) * index);

    if (current == NULL) {
        *(map->items + sizeof(#{N}_entry*) * index) = entry;
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

    map->fillRatio = (float) map->count / (float) map->size;

    return FABRICDB_OK;
}

~

h_template = %Q~\#ifndef __FABRICDB_#{N}_H
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

File.open(cfilename, "w") do |f|
    f.write(c_template)
end

File.open(hfilename, "w") do |f|
    f.write(h_template)
end

puts "Generated #{cfilename} and #{hfilename}"
