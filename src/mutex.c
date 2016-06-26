/******************************************************************
 * FabricDB Library File Locking Implementation
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: June 22, 2016
 * Modified: June 22, 2016
 * Author: Mark Wardle
 * Description:
 *     Implements locking procedures for shared memory in multi-threaded apps.
 *
 ******************************************************************/


#include <assert.h>
#include <pthread.h>

#include "mutex.h"

typedef struct FdbMutex {
    pthread_mutex_t mutex;
    int refCount;
    pthread_t owner;
} FdbMutex;

static int mutexes_initialized = 0;
static FdbMutex mutexes[FDB_MUTEX_COUNT];

void fdb_init_mutexes() {

    pthread_mutexattr_t attr;
    int i;

    if (mutexes_initialized) {
        return;
    }

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    for(i = 0; i < FDB_MUTEX_COUNT; i++) {
        pthread_mutex_init(&(mutexes[i].mutex), &attr);
        mutexes[i].refCount = 0;
        mutexes[i].owner = 0;
    }

    pthread_mutexattr_destroy(&attr);

    mutexes_initialized = 1;
}

void fdb_enter_mutex(int mutexId) {
    int rc;

    assert(mutexes_initialized);
    assert(mutexId < FDB_MUTEX_COUNT);
    FdbMutex *m = &mutexes[mutexId];
    pthread_t self = pthread_self();

    rc = pthread_mutex_lock(&(m->mutex));
    assert(rc == 0);

    if (m->refCount > 0 && m->owner == self) {
        /* Simply increment the count */
        m->refCount++;
    } else {
        /* Set the owner refCount to 1 */
        m->owner = self;
        m->refCount = 1;
    }
}

void fdb_leave_mutex(int mutexId) {
    assert(mutexes_initialized);
    assert(mutexId < FDB_MUTEX_COUNT);

    pthread_t self = pthread_self();
    FdbMutex *m = &(mutexes[mutexId]);

    assert(m->owner == self && m->refCount > 0);
    m->refCount--;
    pthread_mutex_unlock(&m->mutex);

}


#ifdef FABRICDB_TESTING
#include "../test/test_mutex.c"
#endif
