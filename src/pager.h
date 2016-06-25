/*****************************************************************
 * FabricDB Library Pager Header
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: January 29, 2016
 * Modified: January 31, 2016
 * Author: Mark Wardle
 * Description:
 *     Declares low-level file operations.
 *
 ******************************************************************/

#ifndef __FABRICDB_PAGER_H
#define __FABRICDB_PAGER_H

#include <stdint.h>

typedef struct Page {
	uint32_t page_size;
	Page *next;
	uint8_t dirty;
	uint8_t *data;
} Page;

typedef struct PageCache {
	uint32_t cache_size;     /* Max size of cache */
	uint32_t cache_count;    /* Count of currently cached pages */
	uint32_t hash_size;      /* The allocated size of pages field (in pointers) */
	Page *pages;             /* Simple hashmap of cached pages for quick lookup */
} PageCache;

typedef DBState {
	uint32_t file_change_counter;
	uint32_t file_page_count;
	uint32_t file_freepage_count;
	uint32_t schema_cookie;
} DBState;

typedef struct Pragma {
	/* Persistent pragmas
	   Thes can only be set when creating a new database file and can not
	   be altered after a database is fully initialized.
	*/
	uint32_t application_id;
	uint32_t application_version
	uint32_t page_size;
	uint8_t file_format_write_version;n
	uint8_t file_format_read_version;
	uint8_t bytes_reserved;
	uint8_t def_cache_size;
	uint8_t def_auto_vacuum;
	uint8_t def_auto_vacuum_threshold;

	/* Non-persistent pragmas
	   These can be altered at run time and do not persist across
	   database sessions.
	*/
	uint8_t auto_vacuum;
	uint8_t auto_vacuum_threshold;
	uint32_t cache_size; // in pages
} Pragma;

typedef struct Pager {
	const char* path;
	fabricdb_fh *dbfh;
	fabricdb_fh *jafh;
	fabricdb_state dbstate;
	fabricdb_pragma pragma;
	PageCache *cache;
} Pager;


/**
 * Creates a new pager structure.
 *
 * This function does not result in a fully initialized pager object.
 * The reason for this is that a pager object is typically initialized
 * by reading the contents of the database file.  However, when a new
 * database is being created, the pager is initialized first and its
 * values are set in the database file when the database is created.
 *
 * The database file is not opened until fabricdb_pager_init() is called.
 *
 * @param filepath The filepath to the database file.
 * @param pager_ptr OUT A pointer to where a pointer to a pager
 *        structure can be stored.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_create(const char* filepath, Pager **pager_p);

/**
 * Initializes a pager structure from a database file.
 *
 * This function opens the database file and validates it. If the file
 * does not exist or is not a valid FabricDB database file (as understood
 * by this version of the library) an error code will be returned.
 *
 * @param pager A pointer to the pager structure.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_init(Pager *pager);

/**
 * Creates a new database file according to set pragmas.
 *
 * This function results in a fully initialized pager object, unless
 * there is an error.  The file for the database must not already exist
 * or this call will fail.
 *
 * @param pager A pointer to the pager structure.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_init_file(Pager *pager);

/**
 * Frees all memory associated with a pager object and closes and open files.
 *
 * The pointer is no longer valid once this function is called.
 *
 * @param pager_ptr A pointer to a pager structure.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_destroy(Pager *pager);

/**
 * Sets the page size for the database.
 *
 * This function can only be used when creating a new database before
 * the database has been fully initialized.
 *
 * Having an excessivley large page size is likely to be a very poor
 * idea.
 *
 * The default value is 1024.
 * The minimum value is 512.
 *
 * @param size The number of bytes each page contains.
 * @param pager The pager structure for a database connection.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_set_page_size(uint32_t size, Pager *pager);

/**
 * Returns the page size set for the database.
 *
 * Calling this function on an unitialized pager may result in an
 * inaccurate (likely a default) value being returned.
 *
 * @param pager The pager structure for a database connection.
 * @return The size of each page in the database in bytes.
 */
uint32_t fabricdb_pager_get_page_size(Pager *pager);

/**
 * Sets the application version for the file.
 *
 * FabricDB does not use this.  It exists so that user applications
 * can specify an application version.
 *
 * This property is persistent.  As such, the library enforces that it is
 * is set only while creating a new database.
 *
 * @param version The application version number
 * @param pager The pager structure for a database connection.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_set_application_version(uint32_t version, Pager *pager);

/**
* Returns the application version set for the database.
*
* Calling this function on an unitialized pager may result in an
* inaccurate (likely a default) value being returned.
*
* @param pager The pager structure for a database connection.
* @return The user defined application version number.
*/
uint32_t fabricdb_pager_get_application_version(Pager *pager);

/**
 * Sets the application id for the file.
 *
 * FabricDB does not use this.  It exists so that user applications
 * can specify an application id that can be used to identify the file type.
 *
 * This property is persistent.  As such, the library enforces that it is
 * is set only while creating a new database.
 *
 * @param id An application id that should be unique.
 * @param pager The pager structure for a database connection.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_set_application_id(uint32_t version, Pager *pager);

/**
* Returns the application id set for the database.
*
* Calling this function on an unitialized pager may result in an
* inaccurate (likely a default) value being returned.
*
* @param pager The pager structure for a database connection.
* @return The user defined application version id.
*/
uint32_t fabricdb_pager_get_application_id(Pager *pager);


/**
 * Sets the databases file format write version.
 *
 * Currently this value must be 1.  It exists to support future updates
 * to the library.
 *
 * This value may only be set when a new database is being created.
 *
 * @param write_version The value for the write version.
 * @param pager The pager structure for a database connection.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_set_file_format_write_version(uint8_t write_version, Pager *pager);

/**
* Returns the file format write version set for the database.
*
* Calling this function on an unitialized pager may result in an
* inaccurate (likely a default) value being returned.
*
* @param pager The pager structure for a database connection.
* @return The file format write version of the database.
*/
uint8_t fabricdb_pager_get_file_format_write_version(fabricdb_pager *pager);

/**
 * Sets the databases file format read version.
 *
 * Currently this value must be 1.  It exists to support future updates
 * to the library.
 *
 * This value may only be set when a new database is being created.
 *
 * @param read_version The value for the read version.
 * @param pager The pager structure for a database connection.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_set_file_format_read_version(uint8_t read_version, Pager *pager);

/**
* Returns the file format read version set for the database.
*
* Calling this function on an unitialized pager may result in an
* inaccurate (likely a default) value being returned.
*
* @param pager The pager structure for a database connection.
* @return The file format read version of the database.
*/
uint8_t fabricdb_pager_get_file_format_read_version(Pager *pager);

/**
 * Sets the number of reserved bytes for each page.
 *
 * Currently this value should be 0, which is the default.
 *
 * This value may only be set when a new database is being created.
 *
 * @param num_bytes The number of bytes to reserve.
 * @param pager The pager structure for a database connection.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_set_bytes_reserved_space(uint8_t num_bytes, Pager *pager);

/**
* Returns the number of reserved bytes for each pages set for the database.
*
* Calling this function on an unitialized pager may result in an
* inaccurate (likely a default) value being returned.
*
* @param pager The pager structure for a database connection.
* @return The number of reserved bytes for each page of the database.
*/
uint8_t fabricdb_pager_get_bytes_reserved_space(Pager *pager);

/**
 * Sets whether or not the connection should use auto vacuum.
 *
 * A database that uses auto vacuum will remove empty database
 * pages as soon as a set threshold is reached.
 *
 * The default value is 0.
 *
 * @param enabled 0 to disable auto vacuum, >0 to enable it.
 * @param pager The pager structure for a database connection.
 * @return FABRICDB_OK on success, other status code on failure.
 */
int fabricdb_pager_set_auto_vaccum(uint8_t enabled, Pager *pager);

/**
 * Gets whether or not auto vacuum is turned on for the connection.
 *
* Calling this function on an unitialized pager may result in an
* inaccurate (likely a default) value being returned.
*
* @param pager The pager structure for a database connection.
* @return 0 if auto-vacuum is turned off >0 if turned on.
 */
uint8_t fabricdb_pager_get_auto_vacuum(Pager *pager);

/**
* Sets the auto vacuum threshold.
*
* A database that uses auto vacuum will remove empty database
* pages as soon as the given threshold is reached.
*
* The default value is 0.
*
* @param threshold The number of free pages needed to trigger a vacuum operation.
* @param pager The pager structure for a database connection.
* @return FABRICDB_OK on success, other status code on failure.
*/
int fabricdb_pager_set_auto_vaccum_threshold(uint8_t threshold, Pager *pager);

/**
 * Gets the threshold number of pages before auto-vacuuming is initiated.
 *
 * Calling this function on an unitialized pager may result in an
 * inaccurate (likely a default) value being returned.
 *
 * @param pager The pager structure for a database connection.
 * @return The number of free pages the database needs to trigger auto-vacuum.
 */
uint8_t fabricdb_pager_get_auto_vacuum_threshold(Pager *pager);

/**
 * Sets the cache size (in pages) for a connection.
 *
 * More cache = better performance = more memory used.
 *
 * The default value is 2000.
 *
 * @param num_pages The max number of pages to cache.
 * @param pager The pager structure for a database connection.
 * @return FABRIC_OK on success, other status code on failure.
 */
int fabricdb_pager_set_cache_size(uint16_t num_pages, Pager *pager);

/**
 * Gets the cache size constraint for the database.
 *
 * Calling this function on an unitialized pager may result in an
 * inaccurate (likely a default) value being returned.
 *
 * @param pager The pager structure for a database connection.
 * @return The number of pages the database is allowed to cache.
 */
uint16_t fabricdb_pager_get_cache_size(Pager *pager);

#endif /* __FABRICDB_PAGER_H */
