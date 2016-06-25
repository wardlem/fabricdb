/******************************************************************
 * FabricDB Library File Locking Header
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
 *     Describes locking procedures for shared memory in multi-threaded apps.
 *
 ******************************************************************/

#ifndef __FABRICDB_MUTEX_H
#define __FABRICDB_MUTEX_H

/* Mutex ids */
#define FDB_INODE_MUTEX 0

#define FDB_MUTEX_COUNT 1

/**
 * This method must be called to initialize
 * the mutexes used by the library.
 */
void fdb_init_mutexes();

/**
 * Enter a mutex.
 *
 * This function will block the current thread has control of the mutex.
 *
 * It is safe to call this function multiple times by the same thread
 * with the same mutex id, so long as an equal number of calls is made
 * to fdb_leave_mutex.
 *
 * The mutex id should be one of the mutex id constants defined in this
 * file.
 */
void fdb_enter_mutex(int mutexId);

/**
 * Leave a mutex and make it available to another thread.
 *
 * The mutex id should be one of the mutex id constants defined in this
 * file.
 */
void fdb_leave_mutex(int mutexId);

/**
 * Returns 1 if the current thread has the mutex, 0 otherwise.
 */
// int fdb_has_mutex(int mutexId);

#endif /* __FABRICDB_MUTEX_H */
