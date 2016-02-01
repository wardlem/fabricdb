/*****************************************************************
 * FabricDB Library Public API
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 * 
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 * 
 * Created: January 27, 2016
 * Modified: January 28, 2016
 * Author: Mark Wardle
 * Description:
 *     This file provides the public api of the Fabric library.
 *
 ******************************************************************/

#ifndef __FABRICDB_MAIN_H
#define __FABRICDB_MAIN_H

/**
 * Each open FabricDB database is represented by a point to an
 * opaque fabricdb structure.  A fabricdb structure is created
 * with the `fabricdb_open()` or the `fabricdb_create` functions
 * and destroyed with the`fabricdb_close()` function.  Most 
 * database operations require a valid fabricdb object.
 */
typedef struct FabricDB FabricDB;

/**
 * Attempts to create a new FabricDB database instance from
 * an exisitng database file.
 *
 * The given database file must already exist.
 *
 * @param dbname The name of the database file.
 * @param dbptr OUT - The created database object will be
 *        returned at this memory location.
 * @return FABRICDB_OK if the operation is successful,
 *         some other response code on failure.
 */
int fabricdb_open(const char *dbname, FabricDB **dbptr);

/**
 * Attempts to create a new FabricDB database.
 *
 * The given database file must not exist yet.
 *
 * The database file is created, but not written to immediately.
 * This means that this call does not result in a valid database
 * file.  This means that other FabricDB instances will not read or 
 * write to the database file.  This allows the caller to
 * set pragma values for the database using the fabric_pragma_set_*()
 * functions.  Once all pragma values are set, the caller
 * must use the fabric_pragma_write() function to initialize the
 * database file.  At this point, the database is in a valid state
 * and this and other processes may utilize the database file.
 *
 * @param dbname The name of the database file.
 * @param dbptr OUT - The created database object will be
 *        returned at this memory location.
 * @return FABRICDB_OK if the operation is successful,
 *         some other response code on failure.
 */
int fabricdb_create(const char *dbname, FabricDB **dbptr);

/**
 * Closes an open FabricDB database connection and frees all its memory.
 *
 * @param db A pointer to a a fabricdb structure.
 * @return FABRICDB_OK if the operation is successful,
 *         some other response code on failure
 */
int fabricdb_close(FabricDB *db);

#define FABRICDB_OK 0x00000000
#define FABRICDB_EMISUSE 0x01000000
#define FABRICDB_EIO 0x02000000
#define FABRICDB_EMEM 0x03000000

#define FABRICDB_BUSY (FABRICDB_OK | 1)

#define FABRICDB_EMISUSE_NULLPTR (FABRICDB_EMISUSE | 1)
#define FABRICDB_EMISUSE_PRAGMA (FABRICDB_EMISUSE | 2)

#define FABRICDB_ENOENT (FABRICDB_EIO | 1)
#define FABRICDB_EINVALID_FILE (FABRICDB_EIO | 2)

#define FABRICDB_ENOMEM (FABRICDB_EMEM | 1)

#endif /* __FABRICDB_MAIN_H */