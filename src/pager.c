/*****************************************************************
 * FabricDB Library Pager Implementation
 *
 * Copyright (c) 2016, Mark Wardle <mwwardle@gmail.com>
 *
 * This file may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 *
 ******************************************************************
 *
 * Created: January 29, 2016
 * Modified: July 1, 2016
 * Author: Mark Wardle
 * Description:
 *     Defines low-level file operations.
 *
 ******************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>

#include "byteorder.h"
#include "pager.h"
#include "os.h"
#include "mem.h"
#include "fabric.h"
#include "ptrmap.h"

/*******************************************************************
 * FABRICDB HEADER FORMAT
 *
 * A FabricDB data begins with a header that stores information
 * that describes significant aspects of the database file.
 *
 * +-----+------+--------------------------------
 * | pos | size | description
 * +-----+------+--------------------------------
 * |   0 |   16 | The string "FabricDB vers 1"
 * |  16 |    4 | Application ID
 * |  20 |    4 | Application Version
 * |  24 |    4 | Page Size
 * |  28 |    1 | File Format Write Version
 * |  29 |    1 | File Format Read Version
 * |  30 |    1 | Bytes Reserved Space
 * |  31 |    1 | RESERVED / UNUSED
 * |  32 |    4 | File Change Counter
 * |  36 |    4 | File Page Count
 * |  40 |    4 | File Free Page Count
 * |  44 |    4 | Schema Cookie
 * |  48 |    4 | Default Cache Size
 * |  52 |    1 | Default Auto Vacuum Enabled
 * |  53 |    1 | Default Auto Vacuum Threshold
 * |  54 |   46 | Free space for future expansion
 * +-----+------+--------------------------------
 *******************************************************************/
#define FDB_FILE_HEADER_SIZE 100
#define FDB_APPLICATION_ID_OFFSET 16
#define FDB_APPLICATION_VERSION_OFFSET 20
#define FDB_PAGE_SIZE_OFFSET 24
#define FDB_FILE_FORMAT_WRITE_VERSION_OFFSET 28
#define FDB_FILE_FORMAT_READ_VERSION_OFFSET 29
#define FDB_BYTES_RESERVED_OFFSET 30
#define FDB_CHANGE_COUNTER_OFFSET 32
#define FDB_PAGE_COUNT_OFFSET 36
#define FDB_FREE_PAGE_COUNT_OFFSET 40
#define FDB_SCHEMA_COOKIE_OFFSET 44
#define FDB_DEFAULT_CACHE_SIZE_OFFSET 48
#define FDB_DEFAULT_AUTO_VACUUM_OFFSET 52
#define FDB_DEFAULT_AUTO_VACUUM_THRESHOLD_OFFSET 53

#define FDB_MIN_PAGE_SIZE 512

/*****************************************************************
 * The standard header string that starts every FabricDB file.
 *****************************************************************/
static const uint8_t HEADER_STRING[16] =
	{'F','a','b','r','i','c','D','B',' ','v','e','r','s',' ','0','1'};

#define VALID_PAGE_SIZE(v) (v >= 512 && v <= 65536)


/*****************************************************************
 * Page types.
 *****************************************************************/
#define HEADER_PAGE 1  /* The type of the first page of the file */
#define VERTEX_PAGE 2  /* A page of vertex data */
#define EDGE_PAGE 3    /* A page of edge data */
#define STRING_PAGE 4  /* A page for string data */
#define DOC_PAGE 5     /* For document data */
#define ARR_PAGE 6     /* For array data */
#define IND_PAGE 7     /* For indexes */
#define P_PAGE   8     /* Keeps track of page types */
#define CONT_PAGE 9    /* A continuation page */
#define FREE_PAGE 10   /* A free page */

#define UNUSED_PAGE 0  /* A page that has never been used */


/*****************************************************************
 * IO / Paging utility functions
 *****************************************************************/
static int read_page(FileHandle *fh, uint32_t pageno, uint32_t pagesize, uint8_t pageType, Page **pagep) {
    Page *page = NULL;
    int rc;
    *pagep = NULL;

    page = fdbmalloc(sizeof(Page));
    if (page == NULL) {
	    return FABRICDB_ENOMEM;
    }

    page->data = (fdbmalloczero(pagesize));
    if (page->data == NULL) {
        fdbfree(page);
        return FABRICDB_ENOMEM;
    }

    rc = fdb_read(fh, page->data, (pageno - 1) * pagesize, pagesize);
    if (rc != FABRICDB_OK) {
        fdbfree(page->data);
        fdbfree(page);
        return rc;
    }

    page->pageSize = pagesize;
    page->pageNo = pageno;
    page->pageType = pageType;
    page->dirty = 0;
    *pagep = page;

    return FABRICDB_OK;
}

static int write_page(FileHandle *fh, Page *page) {
    return fdb_write(fh, page->data, (page->pageNo - 1) * page->pageSize, page->pageSize);
}

static void free_page(Page *page) {
    fdbfree(page->data);
    fdbfree(page);
}


/*****************************************************************
 * Cache routines.
 *****************************************************************/
static inline int PageCache_create(PageCache *cache, uint32_t size) {
    cache->count = 0;
    return ptrmap_set_size(cache, size * 2);
}

static inline int PageCache_put(PageCache *cache, Page *page) {
    Page *existing = ptrmap_get_or(cache, page->pageNo, NULL);
    assert(existing == NULL);
    return ptrmap_set(cache, page->pageNo, page);
}

static inline void PageCache_deinit(PageCache *cache) {
    ptrmap_deinit(cache);
}

static inline int PageCache_clear(PageCache *cache) {
    /* Free the pages */
    uint32_t index;
    ptrmap_entry* current;

    index = 0;
    while(index < cache->size) {
        current = cache->items[index];
        while (current != NULL) {
            free_page(current->value);
            current = current->next;
        }
        index++;
    }

    return ptrmap_reinit(cache, cache->size);
}

/*******************************************************************
 * Pager creation and initialization routines.
 *******************************************************************/

/* Forward declarations */
static int pager_read_page_types(Pager* pager, Page* frontPage) {
	// uin32_t page_count = pager->page_count;
    return 0;
}
int fdb_pager_create(const char* filepath, Pager **pagerp) {
	*pagerp = NULL;

	Pager *pager = fdbmalloczero(sizeof(Pager));
	if (pager == NULL) {
		return FABRICDB_ENOMEM;
	}

	/* Copy the file path */
	size_t path_len = strlen(filepath);
	char* new_filepath = fdbmalloc(path_len + 1);
	if (new_filepath == NULL) {
		fdbfree(pager);
		return FABRICDB_ENOMEM;
	}
	memcpy(new_filepath, filepath, path_len);
	new_filepath[path_len] = '\0';

	pager->filePath = new_filepath;
	pager->dbfh = NULL;

	/* Defaults */
    pager->dbstate.fileChangeCounter = 0;
    pager->dbstate.filePageCount = 0;
    pager->dbstate.fileFreePageCount = 0;
    pager->dbstate.schemaCookie = 0;

    /* Defaults */
    pager->pragma.applicationId = 0;
    pager->pragma.applicationVersion = 0;
    pager->pragma.pageSize = 1024;
    pager->pragma.fileFormatWriteVersion = 1;
    pager->pragma.fileFormatReadVersion = 1;
    pager->pragma.bytesReserved = 0;
    pager->pragma.defCacheSize = 200;
    pager->pragma.defAutoVacuum = 0;
    pager->pragma.defAutoVacuumThreshold = 0;
    pager->pragma.autoVacuum = 0;
    pager->pragma.autoVacuumThreshold = 0;
    pager->pragma.cacheSize = 200;

	*pagerp = pager;

	return FABRICDB_OK;
}

static int fdb_pager_init_from_file(Pager *pager) {
	int rc = FABRICDB_OK;
	Page *front_page = NULL;
	uint8_t *fp_data;
	int64_t file_size;
	uint8_t header_string[16];
	uint32_t page_size;
    uint8_t num_reserved_bytes;

    page_size = 0;

	/* Make sure the file is at least FABRICDB_MIN_PAGE_SIZE */
    rc = fdb_file_size(pager->dbfh, &file_size);
	if (rc != FABRICDB_OK) {
		goto pager_init_done;
	}
	if (file_size < FDB_MIN_PAGE_SIZE) {
		rc = FABRICDB_EINVALID_FILE;
		goto pager_init_done;
	}

	/* Verify that the 16 byte header string is valid */
    rc = fdb_read(pager->dbfh, header_string, 0, 16);
	if (rc != FABRICDB_OK) {
		goto pager_init_done;
	}
	if (memcmp(header_string, HEADER_STRING, 16) != 0) {
		rc = FABRICDB_EINVALID_FILE;
		goto pager_init_done;
	}

	/* Seems to be a valid FabricDB file
	   Get a shared lock and make sure page size is valid */
    rc = fdb_acquire_shared_lock(pager->dbfh);
	if (rc != FABRICDB_OK) {
		goto pager_init_done;
	}
    rc = fdb_read(pager->dbfh, (uint8_t*) &page_size, FDB_PAGE_SIZE_OFFSET, 4);
	if (rc != FABRICDB_OK){
		page_size = 1; /* indicate lock is acquired */
		goto pager_init_done;
	}
    page_size = letohu32(page_size);

	if (!VALID_PAGE_SIZE(page_size)) {
		rc = FABRICDB_EINVALID_FILE;
		page_size = 1;  /* indicate lock is acquired */
		goto pager_init_done;
	}

    /* The number of reserved bytes needs to be added to the page size */
    rc = fdb_read(pager->dbfh, &num_reserved_bytes, FDB_BYTES_RESERVED_OFFSET, 1);
    if (rc != FABRICDB_OK) {
        goto pager_init_done;
    }

	/* Read the first page */
    rc = read_page(pager->dbfh, 1, page_size + num_reserved_bytes, HEADER_PAGE, &front_page);
	if (rc != FABRICDB_OK) {
		goto pager_init_done;
	}

	fp_data = front_page->data;

	/* Read first page and set values from file */
    pager->dbstate.fileChangeCounter = letohu32(*((uint32_t*)(fp_data + FDB_CHANGE_COUNTER_OFFSET)));
    pager->dbstate.filePageCount = letohu32(*((uint32_t*)(fp_data + FDB_PAGE_COUNT_OFFSET)));
    pager->dbstate.fileFreePageCount = letohu32(*((uint32_t*)(fp_data + FDB_FREE_PAGE_COUNT_OFFSET)));
    pager->dbstate.filePageCount = letohu32(*((uint32_t*)(fp_data + FDB_PAGE_COUNT_OFFSET)));

	pager->pragma.applicationId = letohu32(*((uint32_t*)(fp_data + FDB_APPLICATION_ID_OFFSET)));
	pager->pragma.applicationVersion = letohu32(*((uint32_t*)(fp_data + FDB_APPLICATION_VERSION_OFFSET)));
	pager->pragma.pageSize = page_size;
	pager->pragma.fileFormatWriteVersion = *((uint8_t*)(fp_data + FDB_FILE_FORMAT_WRITE_VERSION_OFFSET));
	pager->pragma.fileFormatReadVersion = *((uint8_t*)(fp_data + FDB_FILE_FORMAT_WRITE_VERSION_OFFSET));
	pager->pragma.bytesReserved = *((uint8_t*)(fp_data + FDB_BYTES_RESERVED_OFFSET));
	pager->pragma.defCacheSize = *((uint8_t*)(fp_data + FDB_DEFAULT_CACHE_SIZE_OFFSET));
	pager->pragma.defAutoVacuum = *((uint8_t*)(fp_data + FDB_DEFAULT_AUTO_VACUUM_OFFSET));
	pager->pragma.defAutoVacuumThreshold = *((uint8_t*)(fp_data + FDB_DEFAULT_AUTO_VACUUM_THRESHOLD_OFFSET));

	pager->pragma.autoVacuum = pager->pragma.defAutoVacuum;
	pager->pragma.autoVacuumThreshold = pager->pragma.defAutoVacuumThreshold;
	pager->pragma.cacheSize = pager->pragma.defCacheSize;

	/* Initialize the cache. */
    rc = PageCache_create(&pager->pageCache, pager->pragma.cacheSize);
	if (rc != FABRICDB_OK) {
		goto pager_init_done;
	}

	/* The rest of the front page (after the header) contains data
	   that describes how every page is used.  After this data is
	   read, every lookup by id in the database takes O(1) time
	   because the exact position in the datafile can be calculated.
	   The downside of this is that the page page has to be maintained. */
	pager_read_page_types(pager, front_page);

	/* Ignore error code */
	PageCache_put(&pager->pageCache, front_page);

	pager_init_done:
	if (page_size != 0) {
		/* Release lock */
		fdb_unlock(pager->dbfh);
	}

	if(rc != FABRICDB_OK){
		/* Clean up memory */
		if (front_page != NULL) {
			free_page(front_page);
		}
		if (pager->dbfh != NULL) {
			fdb_close_file(pager->dbfh);
			pager->dbfh = NULL;
		}
        PageCache_deinit(&pager->pageCache);
	}

	return rc;
}

int fdb_pager_init(Pager *pager) {
	int rc;

    rc = fdb_open_file_rdwr(pager->filePath, &pager->dbfh);
	if (rc != FABRICDB_OK ) {
		return rc;
	}

	return fdb_pager_init_from_file(pager);
}

int fdb_pager_init_file(Pager *pager) {
	int rc;
	uint32_t v32;

	uint8_t *buffer = fdbmalloczero(pager->pragma.pageSize + pager->pragma.bytesReserved);
	if (buffer == NULL) {
		return FABRICDB_ENOMEM;
	}

	/* Create the file */
    rc = fdb_create_file(pager->filePath, &pager->dbfh);
	if (rc != FABRICDB_OK) {
		fdbfree(buffer);
		return rc;
	}

	/* Write persistent pragma values */
	memcpy(buffer, HEADER_STRING, 16);
	v32 = htoleu32(pager->pragma.applicationId);
	memcpy(buffer + FDB_APPLICATION_ID_OFFSET, &v32, 4);
	v32 = htoleu32(pager->pragma.applicationVersion);
	memcpy(buffer + FDB_APPLICATION_VERSION_OFFSET, &v32, 4);
	v32 = htoleu32(pager->pragma.pageSize);
	memcpy(buffer + FDB_PAGE_SIZE_OFFSET, &v32, 4);
	*(buffer + FDB_FILE_FORMAT_WRITE_VERSION_OFFSET) = pager->pragma.fileFormatWriteVersion;
	*(buffer + FDB_FILE_FORMAT_READ_VERSION_OFFSET) = pager->pragma.fileFormatReadVersion;
	*(buffer + FDB_BYTES_RESERVED_OFFSET) = pager->pragma.bytesReserved;
	*(buffer + FDB_DEFAULT_CACHE_SIZE_OFFSET) = pager->pragma.defCacheSize;
	*(buffer + FDB_DEFAULT_AUTO_VACUUM_OFFSET) = pager->pragma.defAutoVacuum;
	*(buffer + FDB_DEFAULT_AUTO_VACUUM_THRESHOLD_OFFSET) = pager->pragma.defAutoVacuumThreshold;

	/* Set page count to 1 */
	v32 = htoleu32(1);
	memcpy(buffer + FDB_PAGE_COUNT_OFFSET, &v32, 4);

	/* Write to disk and sync */
    rc = fdb_write(pager->dbfh, buffer, 0, pager->pragma.pageSize + pager->pragma.bytesReserved);
    if (rc == FABRICDB_OK) {
        rc = fdb_sync(pager->dbfh);
    }
	if (rc != FABRICDB_OK) {
		fdbfree(buffer);
		fdb_close_file(pager->dbfh);
		pager->dbfh = NULL;
		return rc;
	}

	return fdb_pager_init_from_file(pager);
}

void fdb_pager_destroy(Pager *pager) {

}

/*******************************************************************
 * Pragma manipulation.
 *******************************************************************/


 #ifdef FABRICDB_TESTING
 #include "../test/test_pager.c"
 #endif
