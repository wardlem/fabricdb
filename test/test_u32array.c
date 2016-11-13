/*****************************************************************
 * FabricDB Library u32array Interface
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Generated: 2016-07-04
 * Author: Mark Wardle
 *
 ******************************************************************/
#include "test_common.h"

void test_u32array_set_size() {
    u32array arr;

    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Failed to set size", u32array_set_size(&arr, 1) == FABRICDB_OK);
    fdb_assert("Size not set", arr.size == 1);
    fdb_assert("Count is wrong", arr.count == 0);
    fdb_assert("Invalid ptr size", fabricdb_mem_size(arr.data) == sizeof(uint32_t));

    fdb_assert("Failed to set size", u32array_set_size(&arr, 5) == FABRICDB_OK);
    fdb_assert("Size not set", arr.size == 5);
    fdb_assert("Count is wrong", arr.count == 0);
    fdb_assert("Invalid ptr size", fabricdb_mem_size(arr.data) == sizeof(uint32_t) * 5);

    fdb_assert("Could not push value", u32array_push(&arr, 1) == FABRICDB_OK);
    fdb_assert("Could not push value", u32array_push(&arr, 2) == FABRICDB_OK);
    fdb_assert("Could not push value", u32array_push(&arr, 4) == FABRICDB_OK);
    fdb_assert("Could not push value", u32array_push(&arr, 8) == FABRICDB_OK);
    fdb_assert("Could not push value", u32array_push(&arr, 16) == FABRICDB_OK);

    fdb_assert("Size grew", arr.size == 5);
    fdb_assert("Count was not set", arr.count == 5);
    fdb_assert("Ptr size is wrong", fabricdb_mem_size(arr.data) == sizeof(uint32_t) * 5);

    fdb_assert("Values were not set", u32array_get_or(&arr, 0, 0) == 1);
    fdb_assert("Values were not set", u32array_get_or(&arr, 1, 0) == 2);
    fdb_assert("Values were not set", u32array_get_or(&arr, 2, 0) == 4);
    fdb_assert("Values were not set", u32array_get_or(&arr, 3, 0) == 8);
    fdb_assert("Values were not set", u32array_get_or(&arr, 4, 0) == 16);

    fdb_assert("Could not push value", u32array_push(&arr, 0) == FABRICDB_OK);

    fdb_assert("Size did not grow", arr.size > 5);
    fdb_assert("Count was not set", arr.count == 6);
    fdb_assert("Ptr size is wrong", fabricdb_mem_size(arr.data) > sizeof(uint32_t) * 5);

    fdb_assert("Values were not set", u32array_get_or(&arr, 0, 0) == 1);
    fdb_assert("Values were not set", u32array_get_or(&arr, 1, 0) == 2);
    fdb_assert("Values were not set", u32array_get_or(&arr, 2, 0) == 4);
    fdb_assert("Values were not set", u32array_get_or(&arr, 3, 0) == 8);
    fdb_assert("Values were not set", u32array_get_or(&arr, 4, 0) == 16);
    fdb_assert("Values were not set", u32array_get_or(&arr, 5, 1) == 0);


    fdb_assert("Failed to set size", u32array_set_size(&arr, 3) == FABRICDB_OK);
    fdb_assert("Size not set", arr.size == 3);
    fdb_assert("Count is wrong", arr.count == 3);
    fdb_assert("Ptr size is wrong", fabricdb_mem_size(arr.data) == sizeof(uint32_t) * 3);

    fdb_assert("Values were not copied", u32array_get_or(&arr, 0, 0) == 1);
    fdb_assert("Values were not copied", u32array_get_or(&arr, 1, 0) == 2);
    fdb_assert("Values were not copied", u32array_get_or(&arr, 2, 0) == 4);
    fdb_assert("Out of bounds returned old value", u32array_get_or(&arr, 3, 0) == 0);

    u32array_deinit(&arr);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_u32array_has() {
    u32array arr;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Size not initialized to 0", arr.size == 0);
    fdb_assert("Count not initialized to 0", arr.count == 0);
    fdb_assert("Data not initialized to null", arr.data == NULL);

    fdb_assert("Has zero but shouldn't", u32array_has(&arr, 0) == 0);

    fdb_assert("Could not add a value", u32array_set(&arr, 0, 100) == FABRICDB_OK);
    fdb_assert("Could not add a value", u32array_set(&arr, 1, 101) == FABRICDB_OK);
    fdb_assert("Could not add a value", u32array_set(&arr, 2, 102) == FABRICDB_OK);

    fdb_assert("Does not have 0 index", u32array_has(&arr, 0) == 1);
    fdb_assert("Does not have 1 index", u32array_has(&arr, 1) == 1);
    fdb_assert("Does not have 2 index", u32array_has(&arr, 2) == 1);
    fdb_assert("Has 3 index, but should not", u32array_has(&arr, 3) == 0);

    u32array_deinit(&arr);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_u32array_get_or() {
    u32array arr;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Size not initialized to 0", arr.size == 0);
    fdb_assert("Count not initialized to 0", arr.count == 0);
    fdb_assert("Data not initialized to null", arr.data == NULL);

    fdb_assert("Has zero but shouldn't", u32array_get_or(&arr, 0, 1) == 1);

    fdb_assert("Could not add a value", u32array_set(&arr, 0, 100) == FABRICDB_OK);
    fdb_assert("Could not add a value", u32array_set(&arr, 1, 101) == FABRICDB_OK);
    fdb_assert("Could not add a value", u32array_set(&arr, 2, 102) == FABRICDB_OK);

    fdb_assert("Does not have 0 index", u32array_get_or(&arr, 0, 12) == 100);
    fdb_assert("Does not have 1 index", u32array_get_or(&arr, 1, 12) == 101);
    fdb_assert("Does not have 2 index", u32array_get_or(&arr, 2, 12) == 102);
    fdb_assert("Has 3 index, but should not", u32array_get_or(&arr, 3, 12) == 12);

    u32array_deinit(&arr);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_u32array_get_ref() {
    u32array arr;
    uint32_t* v;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Size not initialized to 0", arr.size == 0);
    fdb_assert("Count not initialized to 0", arr.count == 0);
    fdb_assert("Data not initialized to null", arr.data == NULL);

    fdb_assert("Has zero but shouldn't", u32array_get_ref(&arr, 0) == NULL);

    fdb_assert("Could not add a value", u32array_set(&arr, 0, 100) == FABRICDB_OK);
    fdb_assert("Could not add a value", u32array_set(&arr, 1, 101) == FABRICDB_OK);
    fdb_assert("Could not add a value", u32array_set(&arr, 2, 102) == FABRICDB_OK);

    v = u32array_get_ref(&arr, 0);
    fdb_assert("Does not have 0 index", *v == 100);
    v = u32array_get_ref(&arr, 1);
    fdb_assert("Does not have 1 index", *v == 101);
    v = u32array_get_ref(&arr, 2);
    fdb_assert("Does not have 2 index", *v == 102);
    *v = 13;
    fdb_assert("Does not have 2 index", u32array_get_or(&arr, 2, 0) == 13);
    v = u32array_get_ref(&arr, 3);
    fdb_assert("Has 3 index, but should not", v == NULL);

    u32array_deinit(&arr);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_u32array_pop_or() {
    u32array arr;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Could not push value", u32array_push(&arr, 1) == FABRICDB_OK);
    fdb_assert("Could not push value", u32array_push(&arr, 2) == FABRICDB_OK);
    fdb_assert("Could not push value", u32array_push(&arr, 4) == FABRICDB_OK);
    fdb_assert("Could not push value", u32array_push(&arr, 8) == FABRICDB_OK);
    fdb_assert("Could not push value", u32array_push(&arr, 16) == FABRICDB_OK);

    fdb_assert("Count was not set", arr.count == 5);

    fdb_assert("Did not pop value", u32array_pop_or(&arr, 0) == 16);
    fdb_assert("Count not updated", arr.count == 4);
    fdb_assert("Did not pop value", u32array_pop_or(&arr, 0) == 8);
    fdb_assert("Count not updated", arr.count == 3);
    fdb_assert("Did not pop value", u32array_pop_or(&arr, 0) == 4);
    fdb_assert("Count not updated", arr.count == 2);
    fdb_assert("Did not pop value", u32array_pop_or(&arr, 0) == 2);
    fdb_assert("Count not updated", arr.count == 1);
    fdb_assert("Did not pop value", u32array_pop_or(&arr, 0) == 1);
    fdb_assert("Count not updated", arr.count == 0);
    fdb_assert("Did not pop value", u32array_pop_or(&arr, 0) == 0);
    fdb_assert("Count not updated", arr.count == 0);

    u32array_deinit(&arr);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;

}

void test_u32array() {
    fdb_runtest("u32array set size", test_u32array_set_size);
    fdb_runtest("u32array has", test_u32array_has);
    fdb_runtest("u32array get or", test_u32array_get_or);
    fdb_runtest("u32array get ref", test_u32array_get_ref);
    fdb_runtest("u32array pop or", test_u32array_pop_or);
}
