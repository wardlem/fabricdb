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
 * Modified: February 1, 2016
 * Author: Mark Wardle
 * Description:
 *     Defines low-level file operations.
 *
 ******************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "pager.h"
#include "os.h"
#include "mem.h"
#include "fabric.h"

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
#define FABRICDB_FILE_HEADER_SIZE 100
#define FABRICDB_APPLICATION_ID_OFFSET 16
#define FABRICDB_APPLICATION_VERSION_OFFSET 20
#define FABRICDB_PAGE_SIZE_OFFSET 24
#define FABRICDB_FILE_FORMAT_WRITE_VERSION_OFFSET 28
#define FABRICDB_FILE_FORMAT_READ_VERSION_OFFSET 29
#define FABRICDB_BYTES_RESERVED_OFFSET 30
#define FABRICDB_CHANGE_COUNTER_OFFSET 32
#define FABRICDB_PAGE_COUNT_OFFSET 36
#define FABRICDB_FREE_PAGE_COUNT_OFFSET 40
#define FABRICDB_SCHEMA_COOKIE_OFFSET 44
#define FABRICDB_DEFAULT_CACHE_SIZE_OFFSET 48
#define FABRICDB_DEFAULT_AUTO_VACUUM_OFFSET 52
#define FABRICDB_DEFAULT_AUTO_VACUUM_THRESHOLD_OFFSET 53

#define FABRICDB_MIN_PAGE_SIZE 512;

/*****************************************************************
 * The standard header string that starts every FabricDB file.
 *****************************************************************/
static const uint8_t HEADER_STRING[16] =
	{'F','a','b','r','i','c','D','B',' ','v','e','r','s',' ','1','\0'};

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
#define FREE_PAGE 0    /* A free page */


/*****************************************************************
 * IO / Paging utility functions
 *****************************************************************/
static int read_page(uint32_t pageno, uint32_t pagesize, Page **page_p, FileHandle *fh) {
	Page *page = NULL;
	int rc;
	*page_p = NULL;



	page = fdbmalloczero(sizeof(Page) + pagesize);
	if (data == NULL) {
		return FABRICDB_ENOMEM;
	}
	page->data = (uint8_t*)(page + sizeof(Page));

	if (rc = fdb_read(pager->dbfh, page->data, (pageno - 1) * pagesize, pagesize)) {
		fdbfree(page);
		return rc;
	}

	page->page_size = pagesize;

	return FABRICDB_OK;
}

static void free_page(Page *page) {
	if (page->next) {
		fdbfree(page->next);
	}
	fdbfree(page);
}


/*****************************************************************
 * Cache routines.
 *****************************************************************/

static int create_cache(uint32_t cache_size, PageCache **out) {
	PageCache *cache = fdbmalloczero(sizeof(PageCache) + cache_size * sizeof(Page*));
	if (cache == NULL) {
		*out = NULL;
		return FABRICDB_ENOMEM;
	}

	cache->cache_size = cache_size;
	cache->hash_size = buffer_size;
	cache->pages = (Page**)(cache + sizeof(PageCache));
	*out = cache;

	return FABRICDB_OK;
}

static void free_cache(PageCache *cache) {
	int i;
	Page *pages = cache->pages;
	for (i = 0; i < cache->hash_size; i++) {
		if (pages[i]) {
			free_page(pages[i]);
		}
	}
	fdbfree(cache);
}

static void cache_clear_unused(PageCache *cache) {
	/* TODO */
}

static int cache_put(Page *page, PageCache *cache) {
	uint32_t pageno, hash;
	Page *pages = cache->pages;
	Page *current;
	if (cache->cache_size  < 1) {
		/* No cache... this shouldn't happen, but just in case */
		return FABRICDB_CACHE_FULL;
	}

	pageno = page->pageno;
	hash = pageno % cache->hash_size;

	if(cache->cache_count == cache->cache_size) {
		cache_clear_unused(cache);
		if (cache->cache_count == cache->cache_size) {
			return FABRICDB_CACHE_FULL;
		}
	}

	if (pages[hash] == NULL) {
		pages[hash] = page;
		page->cache_count++;
		return FABRICDB_OK;
	}

	/* Make sure it isn't already in the cache */
	current = pages[hash];
	while(current != NULL && current->pageno != pageno) {
		current = current->next;
	}

	if (current == NULL) {
		page->next = pages[hash];
		pages[hash] = page;
		page->cache_count++;
		return FABRICDB_OK;
	}

	/* If we have reached this point, it is definitely an error. */
	return FABRICDB_ECACHE_DUPLICATE_ENTRY;

}

static Page* cache_get(uint32_t pageno, PageCache *cache) {
	Page *page;
	Page *pages = cache->pages;
	if (cache->cache_size  < 1) {
		/* No cache... this shouldn't happen, but it just in case */
		return NULL;
	}

	page = pages[0];
	while(page != NULL && page->pageno != pageno) {
		page = page->next;
	}

	return page;
}


/*******************************************************************
 * Pager creation and initialization routines.
 *******************************************************************/

/* Forward declarations */
static int pager_read_page_types(Page*, Pager*);

int fdb_pager_create(const char* filepath, Pager **pager_p) {
	*pager_p = NULL;

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

	pager->path = new_filepath;
	pager->dbfh = NULL;

	/* Defaults */
	pager->dbstate = {
		0, /* file_change_counter */
		0, /* file_page_count */
		0, /* file_free_page_count */
		0  /* schema_cookie */
	};

	/* Defaults */
	pager->pragma = {
		0,    /* application_id */
		0,    /* application_version */
		1024, /* page_size */
		1,    /* file_format_write_version */
		1,    /* file_format_read_version */
		0,    /* bytes_reserved */
		200,  /* def_cache_size */
		0,    /* def_auto_vacuum */
		0,    /* def_auto_vacuum_threshold */
		0,    /* auto_vacuum */
		0,    /* auto_vacuum_threshold */
		2000  /* cache_size */
	};

	pager->cache = NULL;

	*pager_p = pager;

	return FABRICDB_OK;
}

static int pager_read_page_types(Page* front_page, Pager* pager) {
	uin32_t page_count = pager->page_count;
}

static int fdb_pager_init_from_file(Pager *pager) {
	int rc = FABRICDB_OK;
	Page *front_page = NULL;
	uint8_t *data fp_data;
	int64_t file_size;
	uint8_t header_string[16];
	uint32_t page_size = 0;
	PageCache *cache = NULL;

	/* Make sure the file is at least FABRICDB_MIN_PAGE_SIZE */
	if (rc = fdb_file_size(pager->dbfp, &file_size)) {
		goto pager_init_done;
	}
	if (file_size < FABRICDB_MIN_PAGE_SIZE) {
		rc = FABRICDB_EINVALID_FILE;
		goto pager_init_done;
	}

	/* Verify that the 16 byte header string is valid */
	if (rc = fdb_read(pager->dbfp, header_string, 0, 16)) {
		goto pager_init_done;
	}
	if (memcmp(header_string, HEADER_STRING, 16) != 0) {
		rc = FABRICDB_EINVALID_FILE;
		goto pager_init_done;
	}

	/* Seems to be a valid FabricDB file
	   Get a shared lock and make sure page size is valid */
	if (rc = fdb_get_shared_lock(pager->dbfp)) {
		goto pager_init_done;
	}
	if (rc = fdb_read32(FABRICB_PAGE_SIZE_OFFSET, &page_size, pager->dbfp)){
		page_size = 1; /* indicate lock is acquired */
		goto pager_init_done;
	}
	if (!VALID_PAGE_SIZE(page_size)) {
		rc = FABRICDB_EINVALID_FILE;
		page_size = 1;  /* indicate lock is acquired */
		goto pager_init_done;
	}

	/* Read the first page */
	if (rc = read_page(1, page_size, &front_page, pager->dbfp)) {
		goto pager_init_done;
	}

	fp_data = front_page->data;

	/* Read first page and set values from file */
	pager->state = {
		letohu32(*((uint32_t*)(fp_data + FABRICDB_FILE_CHANGE_COUNTER_OFFSET))),
		letohu32(*((uint32_t*)(fp_data + FABRICDB_PAGE_COUNT_OFFSET))),
		letohu32(*((uint32_t*)(fp_data + FABRICDB_FREE_PAGE_COUNT_OFFSET))),
		letohu32(*((uint32_t*)(fp_data + FABRICDB_PAGE_COUNT_OFFSET)))
	};

	pager->pragma.application_id = letohu32(*((uint32_t*)(fp_data + FABRICDB_APPLICATION_ID_OFFSET)));
	pager->pragma.application_version = letohu32(*((uint32_t*)(fp_data + FABRICDB_APPLICATION_VERSION_OFFSET)));
	pager->pragma.page_size = page_size;
	pager->pragma.file_format_write_version = *((uint8_t*)(fp_data + FABRICDB_FILE_FORMAT_WRITE_VERSION_OFFSET));
	pager->pragma.file_format_read_version = *((uint8_t*)(fp_data + FABRICDB_FILE_FORMAT_WRITE_VERSION_OFFSET));
	pager->pragma.bytes_reserved = *((uint8_t*)(fp_data + FABRICDB_BYTES_RESERVED_OFFSET));
	pager->pragma.def_cache_size = *((uint8_t*)(fp_data + FABRICDB_DEFAULT_CACHE_SIZE_OFFSET));
	pager->pragma.def_auto_vacuum = *((uint8_t*)(fp_data + FABRICDB_DEFAULT_AUTO_VACUUM));
	pager->pragma.def_auto_vacuum_threshold = *((uint8_t*)(fp_data + FABRICDB_DEFAULT_AUTO_VACUUM_THRESHOLD_OFFSET));

	pager->pragma.auto_vacuum = pager->pragma.def_auto_vacuum;
	pager->pragam.auto_vacuum_threshold = pager->pragma.def_auto_vacuum_threshold;
	pager->pragma.cache_size = pager->pragma.def_cache_size;

	/* Initialize the cache. */
	if (rc = create_cache(pager->pragma.cache_size, &pager->cache)) {
		goto pager_init_done;
	}

	/* The rest of the front page (after the header) contains data
	   that describes how every page is used.  After this data is
	   read, every lookup by id in the database takes O(1) time
	   because the exact position in the datafile can be calculated.
	   The downside of this is that the page page has to be maintained. */
	pager_read_page_types(front_page, pager->cache);

	/* Ignore error code */
	cache_set(front_page, pager->cache);

	pager_init_done:
	if (page_size != 0) {
		/* Release lock */
		fdb_unlock(pager->dbfp);
	}

	if(rc != FABRICDB_OK){
		/* Clean up memory */
		if (front_page != NULL) {
			free_page(front_page);
		}
		if (cache != NULL) {
			free_cache(cache);
		}
		if (pager->dbfp != NULL) {
			fdb_close_file(pager->dbfp);
			pager->dbfp = NULL;
		}
	}

	return rc;
}

int fdb_pager_init(Pager *pager) {
	int rc;
	if (rc = fdb_open_file_rdwr(pager->filepath, &pager->dbfp)) {
		return rc;
	}

	return fdb_pager_init_from_file(pager);
}

int fdb_pager_init_file(Pager *pager) {
	int rc;
	uint32_t v32;

	uint8_t *buffer = fdbmalloczero(pager->pragma.page_size);
	if (buffer == NULL) {
		return FABRICDB_ENOMEM
	}

	/* Create the file */
	if (rc = fdb_create_file(pager->filepath, &pager->dbfp)) {
		fdbfree(buffer);
		return rc;
	}

	/* Write persistent pragma values */
	memcpy(buffer, HEADER_STRING, 16);
	v32 = htoleu32(pager->pragma.application_id);
	memcpy(buffer + FABRICDB_APPLICATION_ID_OFFSET, &v32, 4);
	v32 = htoleu32(pager->pragma.application_version);
	memcpy(buffer + FABRICDB_APPLICATION_VERSION_OFFSET, &v32, 4);
	v32 = htoleu32(pager->pragma.page_size);
	memcpy(buffer + FABRICDB_PAGE_SIZE_OFFSET, &v32, 4);
	*(buffer + FABRICDB_FILE_FORMAT_WRITE_VERSION_OFFSET) = pager->pragma.file_format_write_version;
	*(buffer + FABRICDB_FILE_FORMAT_READ_VERSION_OFFSET) = pager->pragma.file_format_read_version;
	*(buffer + FABRICDB_BYTES_RESERVED_OFFSET) = pager->pragma.bytes_reserved;
	*(buffer + FABRICDB_DEFAULT_CACHE_SIZE_OFFSET) = pager->pragma.def_cache_size;
	*(buffer + FABRICDB_DEFAULT_AUTO_VACUUM_OFFSET) = pager->pragma.def_auto_vacuum;
	*(buffer + FABRICDB_DEFAULT_AUTO_VACUUM_THRESHOLD_OFFSET) = pager->pragma.def_auto_vacuum_threshold;

	/* Set page count to 1 */
	v32 = htoleu32(1);
	memcpy(buffer + FABRICDB_PAGE_COUNT_OFFSET, &v32, 4);

	/* Write to disk and sync */
	if (rc = fdb_write(pager->dbfp, buffer, 0, pager->pragma.page_size) ||
		rc = fdb_sync(pager->dbfp)
	) {
		fdbfree(buffer);
		fdb_close(pager->dbfp);
		pager->dbfp = NULL;
		return rc;
	}

	return fdb_pager_init_from_file(pager);
}

int fdb_pager_destroy(Pager *pager) {

}

/*******************************************************************
 * Pragma manipulation.
 *******************************************************************/
