#include <stdio.h>

#include "test_common.h"

// static int tests_run;
// static int tests_passed;

void all_tests() {
    fdb_runsuite("Mem", test_mem);
    fdb_runsuite("Byte Order", test_byteorder);
    fdb_runsuite("Mutex", test_mutex);
    fdb_runsuite("OS UNIX", test_os_unix);
    fdb_runsuite("Pager", test_pager);
}

int tests_passed = 0;
int tests_run = 0;

int main(int argc, char** argv) {

    printf("Running test suite...\n");

    all_tests();

    printf("\nFinished testing with %s%d of %d%s passed tests.\n", tests_run == tests_passed ? GREEN : RED, tests_passed, tests_run, PLAIN);

    return tests_run != tests_passed;
}
