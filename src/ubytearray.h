#ifndef __FABRICDB_ubytearray_H
#define __FABRICDB_ubytearray_H

typedef struct ubytearray {
    uint32_t size;       /* Max size of cache before resizing */
    uint32_t count;      /* Count of currently cached items */
    uint8_t* data;          /* Array of items */
} ubytearray;

int ubytearray_set_size(ubytearray* arr, uint32_t size);
void ubytearray_deinit(ubytearray* arr);
int ubytearray_reinit(ubytearray* arr, uint32_t size);
int ubytearray_has(ubytearray* arr, uint32_t index);
uint8_t ubytearray_get_or(ubytearray* arr, uint32_t index, uint8_t def);
uint8_t* ubytearray_get_ref(ubytearray* arr, uint32_t index);
int ubytearray_set(ubytearray* arr, uint32_t index, uint8_t value);
int ubytearray_push(ubytearray* arr, uint8_t value);
uint8_t ubytearray_pop_or(ubytearray* arr, uint8_t def);


#endif /* __FABRICDB_ubytearray_H */
