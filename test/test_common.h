#ifndef __FABRICDB_TESTINGCOMMON_H
#define __FABRICDB_TESTINGCOMMON_H


#include <stdio.h>

#define PLAIN "\033[0m"
#define BLUE "\033[1;34m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define GRAY "\033[0;37m"

extern int tests_passed;
extern int tests_run;

#define fdb_assert(message, test) do { if (!(test)){ printf("%sFAILED(%d):  %s%s\n",RED, __LINE__,message, PLAIN); return; }} while (0)
#define fdb_runtest(name, test) do { printf("    %s%s ",GRAY,name); \
                                test(); tests_run++; \
                              } while (0)
#define fdb_runsuite(name, test) do { printf("\n%s%s\n", BLUE, name); test();} while (0)
#define fdb_passed do { printf("%sPASSED%s\n", GREEN, PLAIN); tests_passed++; return; } while (0)


void test_mem();
void test_byteorder();
void test_mutex();
void test_os_unix();
void test_pager();

#endif /* __FABRICDB_TESTINGCOMMON_H */
