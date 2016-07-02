#include <stdint.h>
#include <stdlib.h>
#include "ptrmap.h"
#include "fabric.h"
#include "mem.h"

int ptrmap_set_size(ptrmap* map, uint32_t size) {
    uint32_t indexOld;
    uint32_t indexNew;
    uint32_t oldSize;
    ptrmap_entry** oldItems;
    ptrmap_entry** newItems;
    ptrmap_entry* current;
    ptrmap_entry* next;


    oldItems = map->items;
    newItems = (ptrmap_entry**)fdbmalloczero(sizeof(ptrmap_entry*) * size);
    if (newItems == NULL) {
        return FABRICDB_ENOMEM;
    }
    oldSize = map->size;


    if (map->items != NULL) {

        indexOld = 0;
        while (indexOld < oldSize) {
            current = *(oldItems + sizeof(ptrmap_entry) * indexOld);
            while(current != NULL) {
                next = current->next;
                indexNew = current-> key % size;
                current->next = *(newItems + sizeof(ptrmap_entry*) * indexNew);
                *(newItems + sizeof(ptrmap_entry) * indexNew) = current;
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

static void ptrmap_free_item(ptrmap_entry* item) {
    if (item == NULL) {
        return;
    }

    if(item->next != NULL) {
        ptrmap_free_item(item->next);
    }

    fdbfree(item);
}

void ptrmap_deinit(ptrmap* map) {
    ptrmap_entry* current;
    uint32_t index;

    if (map->items == NULL) {
        return;
    }

    index = 0;
    while(index < map->size) {
        current = *(map->items + sizeof(ptrmap_entry*) * index);
        ptrmap_free_item(current);
        index++;
    }

    fdbfree(map->items);
    map->items = NULL;
    map->size = 0;
}

int ptrmap_reinit(ptrmap* map, uint32_t size) {
    ptrmap_deinit(map);
    return ptrmap_set_size(map, size);
}

int ptrmap_has(ptrmap* map, uint32_t key) {
    uint32_t index;
    ptrmap_entry* current;

    index = key % map->size;
    current = *(map->items + sizeof(ptrmap_entry*) * index);

    while(current != NULL) {
        if(current->key == key) {
            return 1;
        }
        current = current->next;
    }

    return 0;
}

void* ptrmap_get_or(ptrmap* map, uint32_t key, void* def) {
    uint32_t index = key % map->size;
    ptrmap_entry* current = *(map->items + sizeof(ptrmap_entry*) * index);

    while(current != NULL) {
        if(current->key == key) {
            return current->value;
        }
        current = current->next;
    }

    return def;
}

void** ptrmap_get_ref(ptrmap* map, uint32_t key) {
    void** out = NULL;
    uint32_t index = key % map->size;
    ptrmap_entry* current = *(map->items + sizeof(ptrmap_entry*) * index);

    while(current != NULL) {
        if(current->key == key) {
            out = &current->value;
            break;
        }
        current = current->next;
    }

    return out;
}

int ptrmap_set(ptrmap* map, uint32_t key, void* value) {
    ptrmap_entry* entry;
    ptrmap_entry* current;
    uint32_t index;
    int rc = FABRICDB_OK;

    entry = fdbmalloc(sizeof(ptrmap_entry));
    if (entry == NULL) {
        return FABRICDB_ENOMEM;
    }

    if (map->resizeRatio < map->fillRatio) {
        rc = ptrmap_set_size(map, map->size * 2 + 1);
        if (rc != FABRICDB_OK) {
            fdbfree(entry);
            return rc;
        }
    }

    entry->key = key;
    entry->value = value;
    entry->next = NULL;

    index = key % map->size;
    current = *(map->items + sizeof(ptrmap_entry*) * index);

    if (current == NULL) {
        *(map->items + sizeof(ptrmap_entry*) * index) = entry;
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

