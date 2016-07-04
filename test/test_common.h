#ifndef __FABRICDB_TESTINGCOMMON_H
#define __FABRICDB_TESTINGCOMMON_H


#include <stdio.h>

#define PLAIN "\033[0m"
#define BLUE "\033[1;34m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define GRAY "\033[1;37m"

extern int tests_passed;
extern int tests_run;
extern int asserts_run;

#define fdb_assert(message, test) do { if (!(test)){ printf("%sFAILED(%d):  %s%s\n",RED, __LINE__,message, PLAIN); return; } asserts_run++;} while (0)
#define fdb_runtest(name, test) do { asserts_run = 0; printf("    %s%s %s",GRAY,name,PLAIN); \
                                test(); tests_run++; \
                              } while (0)
#define fdb_runsuite(name, test) do { printf("\n%s%s%s\n", BLUE, name, PLAIN); test();} while (0)
#define fdb_passed do { printf("%sPASSED (%d)%s\n", GREEN, asserts_run, PLAIN); tests_passed++; return; } while (0)


void test_mem();
void test_byteorder();
void test_mutex();
void test_os_unix();
void test_u8array();
void test_u32array();
void test_ptrmap();
void test_pager();

#endif /* __FABRICDB_TESTINGCOMMON_H */
