/*****************************************************************
 * FabricDB Library Memory Management and Allocation
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: January 29, 2016
 * Modified: July 3, 2016
 * Author: Mark Wardle
 * Description:
 *     This file implements memory allocation and handling routines
 *     for the database.
 * Credit: Based on the zmalloc implementation for Redis
 *
 ******************************************************************/

#include <stdlib.h>
#include <string.h>

#include "mem.h"

#define FABRICDB_MEM_PREFIX_SIZE (sizeof(size_t))

/**
 * Total amount of memory used by the library.
 */
static size_t used_memory = 0;

static inline void update_memused_alloc(size_t num_bytes) {
	used_memory += num_bytes;
}

static inline void update_memused_free(size_t num_bytes) {
	used_memory -= num_bytes;
}

size_t fabricdb_mem_used() {
	return used_memory;
}

void* fabricdb_malloc(size_t num_bytes) {
    if (num_bytes == 0) {
        return NULL;
    }
    
	void *ptr = malloc(num_bytes + FABRICDB_MEM_PREFIX_SIZE);
	if (ptr == NULL) {
		return ptr;
	}

	*((size_t*)ptr) = num_bytes;
	update_memused_alloc(num_bytes + FABRICDB_MEM_PREFIX_SIZE);
	return (uint8_t*)ptr + FABRICDB_MEM_PREFIX_SIZE;
}

void* fabricdb_malloc_zero(size_t num_bytes) {
	void *ptr = fabricdb_malloc(num_bytes);
	if (ptr == NULL) {
		return ptr;
	}

	memset(ptr, 0, num_bytes);

	return ptr;
}

void *fabricdb_realloc(void *ptr, size_t num_bytes) {
	size_t old_num_bytes = fabricdb_mem_size(ptr);
	void *realptr = (uint8_t*)ptr - FABRICDB_MEM_PREFIX_SIZE;
	void *newptr = realloc(realptr, num_bytes + FABRICDB_MEM_PREFIX_SIZE);

	if (newptr == NULL) {
		return newptr;
	}

	*((size_t*)newptr) = num_bytes;

	update_memused_free(old_num_bytes);
	update_memused_alloc(num_bytes);

	return (uint8_t*)newptr + FABRICDB_MEM_PREFIX_SIZE;
}

void *fabricdb_realloc_zero(void *ptr, size_t num_bytes) {
	size_t old_num_bytes = fabricdb_mem_size(ptr);
	void *newptr = fabricdb_realloc(ptr, num_bytes);
	if (newptr == NULL || old_num_bytes > num_bytes) {
		return newptr;
	}

	memset((uint8_t*)newptr + old_num_bytes, 0, num_bytes - old_num_bytes);

	return newptr;
}

void fabricdb_free(void *ptr) {
	if (ptr == NULL) {
		return;
	}

	void *realptr = (uint8_t*)ptr - FABRICDB_MEM_PREFIX_SIZE;
	size_t num_bytes = *((size_t*)realptr);
	free(realptr);
	update_memused_free(num_bytes + FABRICDB_MEM_PREFIX_SIZE);
}

size_t fabricdb_mem_size(void *ptr) {
    if (ptr == NULL) {
        return 0;
    }
	void *realptr = (uint8_t*)ptr - FABRICDB_MEM_PREFIX_SIZE;
	return *((size_t*)realptr);
}

#ifdef FABRICDB_TESTING
#include "../test/test_mem.c"
#endif
