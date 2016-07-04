#ifndef __FABRICDB_u32array_H
#define __FABRICDB_u32array_H

typedef struct u32array {
    uint32_t size;       /* Max size of cache before resizing */
    uint32_t count;      /* Count of currently cached items */
    uint32_t* data;          /* Array of items */
} u32array;

int u32array_set_size(u32array* arr, uint32_t size);
void u32array_deinit(u32array* arr);
int u32array_reinit(u32array* arr, uint32_t size);
int u32array_has(u32array* arr, uint32_t index);
uint32_t u32array_get_or(u32array* arr, uint32_t index, uint32_t def);
uint32_t* u32array_get_ref(u32array* arr, uint32_t index);
int u32array_set(u32array* arr, uint32_t index, uint32_t value);
int u32array_push(u32array* arr, uint32_t value);
uint32_t u32array_pop_or(u32array* arr, uint32_t def);


#endif /* __FABRICDB_u32array_H */
