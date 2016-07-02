#ifndef __FABRICDB_ptrmap_H
#define __FABRICDB_ptrmap_H
typedef struct ptrmap_entry {
    uint32_t key;
    void* value;
    struct ptrmap_entry* next;
} ptrmap_entry;

typedef struct ptrmap {
    uint32_t size;       /* Max size of map before resizing */
    uint32_t count;      /* Count of currently mapd items */
    float resizeRatio;   /* When to resize */
    float fillRatio;     /* count / size */
    ptrmap_entry** items;  /* Array of item buckets */
} ptrmap;

int ptrmap_set_size(ptrmap* map, uint32_t size);
void ptrmap_deinit(ptrmap* map);
int ptrmap_reinit(ptrmap* map, uint32_t size);
int ptrmap_has(ptrmap* map, uint32_t key);
void* ptrmap_get_or(ptrmap* map, uint32_t key, void* def);
void** ptrmap_get_ref(ptrmap* map, uint32_t key);
int ptrmap_set(ptrmap* map, uint32_t key, void* value);

#endif /* __FABRICDB_ptrmap_H */
