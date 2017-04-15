#include <stdio.h>

#include "test_common.h"

// static int tests_run;
// static int tests_passed;

void all_tests() {
    fdb_runsuite("Mem", test_mem);
    fdb_runsuite("Byte Order", test_byteorder);
    fdb_runsuite("Mutex", test_mutex);
    fdb_runsuite("OS UNIX", test_os_unix);
    fdb_runsuite("u32array", test_u32array);
    fdb_runsuite("u8array", test_u8array);
    fdb_runsuite("ptrmap", test_ptrmap);
    fdb_runsuite("Pager", test_pager);
    fdb_runsuite("Property", test_property);
    fdb_runsuite("FString", test_fstring);
    fdb_runsuite("Symbol", test_symbol);
    fdb_runsuite("Vertex", test_vertex);
}

int tests_passed = 0;
int tests_run = 0;
int asserts_run = 0;

#define DO_EXPAND(VAL)  VAL ## 1
#define EXPAND(VAL)     DO_EXPAND(VAL)

#define DO_TEST(test) do { \
                                test(); \
                              } while (0)

int main(int argc, char** argv) {

    printf("Running test suite...\n");
    // fdb_init_mutexes();

    all_tests();

    printf("\nFinished testing with %s%d of %d%s passed tests.\n", tests_run == tests_passed ? GREEN : RED, tests_passed, tests_run, PLAIN);

    return tests_run != tests_passed;
}
