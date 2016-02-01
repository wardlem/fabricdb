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
 * Modified: January 29, 2016
 * Author: Mark Wardle
 * Description:
 *     This file declares allocation and handling routines
 *     for the database.
 *
 ******************************************************************/

#ifndef __FABRICDB_MEM_H
#define __FABRICDB_MEM_H

#include <stdlib.h>

/**
 * Attempts to allocate the specified number of bytes.
 *
 * Works the same as malloc except that it tracks the amount of memory
 * allocated for each pointer as well as the total amount of memory used
 * by the library.
 *
 * @param num_bytes The number of bytes to allocate.
 * @return A pointer to the allocated memory or NULL on failure.
 */
void *fabricdb_malloc(size_t num_bytes);

/**
 * Attempts to allocate the specified number of bytes and zeros the memory.
 *
 * @see fabricdb_malloc
 *
 * @param num_bytes The number of bytes to allocate.
 * @return A pointer to the allocated memory or NULL on failure.
 */
void *fabricdb_malloc_zero(size_t num_bytes);

/**
 * Attempts to reallocate a ptr to the specified number of bytes
 *
 * Works the same as realloc except that it tracks the amount memory allocated
 * for each pointer as well as the total amount of memory used by the library.
 *
 * @param ptr The old pointer which will no longer be valid.
 * @param num_bytes The number of bytes the returned pointer should allocate for.
 * @return A pointer to the allocated memory or NULL on failure.
 */
void *fabricdb_realloc(void *ptr, size_t num_bytes);

/**
 * Attempts to reallocate a ptr to the specified number of bytes and zeros
 * out all additional memory that is allocated.
 *
 * @see baricdb_realloc
 *
 * @param ptr The old pointer which will no longer be valid.
 * @param num_bytes The number of bytes the returned pointer should allocate for.
 * @return A pointer to the allocated memory or NULL on failure.
 */
void *fabricdb_realloc_zero(void *ptr, size_t num_bytes);

/**
 * Frees the memory associated with a pointer.
 *
 * Works the same as free except that it tracks the amount of memory that is
 * freed.
 *
 * @param ptr A pointer to memory previously returned by fabricdb_malloc[_zero]().
 * @return void
 */
void fabricdb_free(void* ptr);

/**
 * Returns the amount of memory used by a pointer.
 *
 * @param ptr A pointer to memory previously returned by fabricdb_malloc[_zeor]().
 * @return The number of bytes allocated for the ptr.
 */
size_t fabricdb_mem_size(void* ptr);

/**
 * Returns the total amount of memory used by the library.
 *
 * @return The number of bytes the library has allocated.
 */
size_t fabricdb_mem_used();

/** Shorthand macros for common memory functions */
#define fdbmalloc(n) fabricdb_malloc(n)
#define fdbmalloczero(n) fabricdb_malloc_zero(n)
#define fdbrealloc(p,n) fabricdb_realloc(p,n)
#define fdbrealloczero(p,n) fabricdb_realloc_zero(p,n)
#define fdbfree(p) fabricdb_free(p)

#endif /* __FABRICDB_MEM_H */