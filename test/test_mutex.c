#include <unistd.h>
#include <errno.h>
#include "test_common.h"

void test_init_mutexes() {
    fdb_init_mutexes();
    fdb_assert("Mutexes were not initialized", mutexes_initialized == 1);

    fdb_passed;
}

static int mutex_test;
static int t_1result;
static int t_2result;

void *thread_increment_test_1(void *t) {
    sleep(1);

    fdb_enter_mutex(FDB_INODE_MUTEX);

    mutex_test++;
    t_1result = mutex_test == 2;

    fdb_leave_mutex(FDB_INODE_MUTEX);

    pthread_exit((void *) 0);
}

void *thread_increment_test_2(void *t) {

    fdb_enter_mutex(FDB_INODE_MUTEX);
    sleep(2);

    mutex_test++;
    t_2result = mutex_test == 1;

    fdb_leave_mutex(FDB_INODE_MUTEX);

    pthread_exit((void *) 0);
}

char* joinReturnCodeString(int code) {
    switch(code) {
        case 0:
            return "SUCCESS";
        case EDEADLK:
            return "EDEADLK";
        case EINVAL:
            return "EINVAL";
        case ESRCH:
            return "ESRCH";
    }

    return "???";
}

void test_enter_mutex() {
    mutex_test = 0;
    pthread_t th1;
    pthread_t th2;
    pthread_attr_t attr;
    int rc;

    fdb_init_mutexes();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&th1, &attr, thread_increment_test_1, (void *) 0);
    pthread_create(&th2, &attr, thread_increment_test_2, (void *) 0);

    pthread_attr_destroy(&attr);

    rc = pthread_join(th1, NULL);
    fdb_assert("Thread one exited with bad return code", !rc);
    rc = pthread_join(th2, NULL);
    fdb_assert("Thread two exited with bad return code", !rc);

    fdb_assert("Mutex did not hold", t_1result);
    fdb_assert("Mutex did not hold", t_2result);

    fdb_passed;

}

void test_mutex() {
    fdb_runtest("Init mutexes", test_init_mutexes);
    fdb_runtest("Enter / Leave Mutex", test_enter_mutex);
}
