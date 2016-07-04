#ifndef __FABRICDB_u8array_H
#define __FABRICDB_u8array_H

typedef struct u8array {
    uint32_t size;       /* Max size of cache before resizing */
    uint32_t count;      /* Count of currently cached items */
    uint8_t* data;          /* Array of items */
} u8array;

int u8array_set_size(u8array* arr, uint32_t size);
void u8array_deinit(u8array* arr);
int u8array_reinit(u8array* arr, uint32_t size);
int u8array_has(u8array* arr, uint32_t index);
uint8_t u8array_get_or(u8array* arr, uint32_t index, uint8_t def);
uint8_t* u8array_get_ref(u8array* arr, uint32_t index);
int u8array_set(u8array* arr, uint32_t index, uint8_t value);
int u8array_push(u8array* arr, uint8_t value);
uint8_t u8array_pop_or(u8array* arr, uint8_t def);


#endif /* __FABRICDB_u8array_H */
