#include "test_common.h"
void test_ptrmap_set_size() {
    ptrmap map;
    int memUsed;
    int testV;

    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);
    fdb_assert("Count is set", map.count == 0);

    fdb_assert("Set size failed", ptrmap_set_size(&map, 5) == FABRICDB_OK);
    fdb_assert("Did not allocate memory", fabricdb_mem_used() > 0);
    fdb_assert("Did not set size", map.size == 5);
    fdb_assert("Set count", map.count == 0);

    memUsed = fabricdb_mem_used();

    fdb_assert("Set size failed", ptrmap_set_size(&map, 10) == FABRICDB_OK);
    fdb_assert("Did not allocate memory", fabricdb_mem_used() > 0);
    fdb_assert("Old data not freed", fabricdb_mem_used() - (5 * sizeof(void*)) == memUsed);
    fdb_assert("Did not set size", map.size == 10);
    fdb_assert("Set count", map.count == 0);

    testV = 1+map.size;
    fdb_assert("Insert failed", ptrmap_set(&map, 1, (void*)2) == FABRICDB_OK);
    fdb_assert("Insert failed", ptrmap_set(&map, 3, (void*)8) == FABRICDB_OK);
    fdb_assert("Insert failed", ptrmap_set(&map, testV, (void*)1) == FABRICDB_OK);

    fdb_assert("Does not have value 1", ptrmap_has(&map, 1) == 1);
    fdb_assert("Does not have value 3", ptrmap_has(&map, 3) == 1);
    fdb_assert("Does not have testV", ptrmap_has(&map,testV) == 1);
    fdb_assert("Has value 2", ptrmap_has(&map, 2) == 0);
    fdb_assert("Was resized", map.size == 10);
    fdb_assert("Count not set", map.count == 3);

    fdb_assert("Set size failed", ptrmap_set_size(&map, 20) == FABRICDB_OK);
    fdb_assert("Size not set", map.size == 20);
    fdb_assert("Count was changed", map.count == 3);

    fdb_assert("Does not have value 1", ptrmap_has(&map, 1) == 1);
    fdb_assert("Does not have value 3", ptrmap_has(&map, 3) == 1);
    fdb_assert("Does not have value testV", ptrmap_has(&map,testV) == 1);
    fdb_assert("Has value 2", ptrmap_has(&map, 2) == 0);

    fdb_assert("Set size failed", ptrmap_set_size(&map, 3) == FABRICDB_OK);
    fdb_assert("Size not set", map.size == 3);
    fdb_assert("Count was changed", map.count == 3);

    fdb_assert("Does not have value 1", ptrmap_has(&map, 1) == 1);
    fdb_assert("Does not have value 3", ptrmap_has(&map, 3) == 1);
    fdb_assert("Does not have value testV", ptrmap_has(&map,testV) == 1);
    fdb_assert("Has value 2", ptrmap_has(&map, 2) == 0);

    /* trigger auto resize */
    fdb_assert("Insert failed", ptrmap_set(&map, 5, (void*)32) == FABRICDB_OK);
    fdb_assert("Size not set", map.size > 3);
    fdb_assert("Count was changed", map.count == 4);

    fdb_assert("Does not have value 1", ptrmap_has(&map, 1) == 1);
    fdb_assert("Does not have value 3", ptrmap_has(&map, 3) == 1);
    fdb_assert("Does not have value testV", ptrmap_has(&map,testV) == 1);
    fdb_assert("Does not have value 5", ptrmap_has(&map,5) == 1);
    fdb_assert("Has value 2", ptrmap_has(&map, 2) == 0);

    ptrmap_deinit(&map);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_ptrmap_get_ref() {
    ptrmap map;
    void** v;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);
    fdb_assert("Count is set", map.count == 0);

    fdb_assert("Resize failed", ptrmap_set_size(&map, 3) == FABRICDB_OK);

    fdb_assert("Insert failed", ptrmap_set(&map, 1, (void*)2) == FABRICDB_OK);
    fdb_assert("Insert failed", ptrmap_set(&map, 3, (void*)8) == FABRICDB_OK);
    fdb_assert("Insert failed", ptrmap_set(&map, 9, (void*)1) == FABRICDB_OK);

    /* force a conflict */
    fdb_assert("Resize failed", ptrmap_set_size(&map, 3) == FABRICDB_OK);
    fdb_assert("Resize did not set size", map.size == 3);

    v = ptrmap_get_ref(&map, 1);
    fdb_assert("Did not return correct reference", v != NULL);
    fdb_assert("Did not return correct value", *v == (void*)2);
    v = ptrmap_get_ref(&map, 3);
    fdb_assert("Did not return correct reference", v != NULL);
    fdb_assert("Did not return correct value", *v == (void*)8);
    v = ptrmap_get_ref(&map, 9);
    fdb_assert("Did not return correct reference", v != NULL);
    fdb_assert("Did not return correct value", *v == (void*)1);
    *v = (void*)3;
    fdb_assert("Did not update reference value", ptrmap_get_or(&map, 9, 0) == (void*)3);
    v = ptrmap_get_ref(&map, 2);
    fdb_assert("Did not return null", v == NULL);
    v = ptrmap_get_ref(&map, 6);
    fdb_assert("Did not return null", v == NULL);


    ptrmap_deinit(&map);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_ptrmap() {
    fdb_runtest("ptrmap set size", test_ptrmap_set_size);
    fdb_runtest("ptrmap get ref", test_ptrmap_get_ref);
}
