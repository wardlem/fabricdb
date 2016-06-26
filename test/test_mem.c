#include "test_common.h"

typedef struct MemTestStruct {
    int a;
    int b;
    int c;
} MemTestStruct;

void test_update_memused_alloc() {
    used_memory = 0;
    update_memused_alloc(10);
    fdb_assert("Failed to update used_memory with positive value", fabricdb_mem_used() == 10);
    update_memused_alloc(-6);
    fdb_assert("Failed to update used memory with negative value", fabricdb_mem_used() == 4);
    fdb_passed;
}

void test_update_memused_free() {
    used_memory = 100;
    update_memused_free(10);
    fdb_assert("Failed to update used_memory with positive value", fabricdb_mem_used() == 90);
    update_memused_free(-6);
    fdb_assert("Failed to update used memory with negative value", fabricdb_mem_used() == 96);
    fdb_passed;
}

void test_fabricdb_malloc() {
    used_memory = 0;
    int s = sizeof(MemTestStruct) + FABRICDB_MEM_PREFIX_SIZE;

    MemTestStruct *t1 = fdbmalloc(sizeof(MemTestStruct));
    fdb_assert("Returned null pointer", t1);
    fdb_assert("Did not update memory used", fabricdb_mem_used() == s);

    MemTestStruct *t2 = fdbmalloc(sizeof(MemTestStruct));
    fdb_assert("Returned null pointer", t2);
    fdb_assert("Did not update memory used", fabricdb_mem_used() == s * 2);

    fdbfree(t2);
    fdb_assert("Did not update memory used", fabricdb_mem_used() == s);
    fdbfree(t1);
    fdb_assert("Did not update memory used", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_fabricdb_realloc() {
    used_memory = 0;
    int s1 = 3200 + FABRICDB_MEM_PREFIX_SIZE;
    int s2 = 4300 + FABRICDB_MEM_PREFIX_SIZE;

    void* t1 = fdbmalloc(3200);
    fdb_assert("Returned null pointer", t1);
    fdb_assert("Did not update memory used", fabricdb_mem_used() == s1);

    t1 = fabricdb_realloc(t1, 4300);
    fdb_assert("Returned null pointer", t1);
    fdb_assert("Did not update memory used", fabricdb_mem_used() == s2);

    fdbfree(t1);
    fdb_assert("Did not update memory used", fabricdb_mem_used() == 0);

    fdb_passed;
}


void test_mem() {
    fdb_runtest("Update Memused Alloc", test_update_memused_alloc);
    fdb_runtest("Update Memused Free", test_update_memused_free);
    fdb_runtest("FabricDB Malloc/Free", test_fabricdb_malloc);
    fdb_runtest("FabricDB Realloc", test_fabricdb_realloc);

}
