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
static FdbMutex mutexes[1];

void fdb_init_mutexes() {
    if (mutexes_initialized) {
        return;
    }

    int i;
    for(i = 0; i < FDB_MUTEX_COUNT; i++) {
        pthread_mutex_init(&(mutexes[i].mutex), NULL);
        mutexes[i].refCount = 0;
        mutexes[i].owner = 0;
    }

    mutexes_initialized = 1;
}

void fdb_enter_mutex(int mutexId) {
    assert(mutexes_initialized);
    assert(mutexId < FDB_MUTEX_COUNT);
    FdbMutex m = mutexes[mutexId];
    pthread_t self = pthread_self();
    if (m.refCount > 0 && m.owner == self) {
        /* Simply increment the count */
        m.refCount++;
    } else {
        /* Get the mutex and set refCount to 1 */
        pthread_mutex_lock(&m.mutex);
        assert(m.refCount == 0);
        m.owner = self;
        m.refCount = 1;
    }
}

void fdb_leave_mutex(int mutexId) {
    assert(mutexes_initialized);
    assert(mutexId < FDB_MUTEX_COUNT);

    pthread_t self = pthread_self();
    FdbMutex m = mutexes[mutexId];

    assert(m.owner == self && m.refCount > 0);

    m.refCount--;
    if (m.refCount == 0) {
        pthread_mutex_unlock(&m.mutex);
    }
}
