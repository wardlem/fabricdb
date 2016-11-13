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
 * Modified: July 4, 2016
 * Author: Mark Wardle
 * Description:
 *     Defines low-level file operations.
 *
 ******************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>


#include "byteorder.h"
#include "pager.h"
#include "os.h"
#include "mem.h"
#include "fabric.h"
#include "ptrmap.h"
#include "u8array.h"
#include "u32array.h"

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
 * |  54 |    2 | RESERVED / UNUSED
 * |  56 |   46 | Free space for future expansion
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
#define FDB_DEFAULT_PAGE_SIZE 1024
#define FDB_DEFAULT_CACHE_SIZE 200

/*****************************************************************
 * The standard header string that starts every FabricDB file.
 *****************************************************************/
static const uint8_t HEADER_STRING[16] =
    {'F','a','b','r','i','c','D','B',' ','v','e','r','s',' ','0','1'};

#define VALID_PAGE_SIZE(v) (v >= 512 && v <= 65536)
#define VALID_FILE_FORMAT_WRITE_VERSION(v) (v == 1)
#define VALID_FILE_FORMAT_READ_VERSION(v) (v == 1)
#define VALID_CACHE_SIZE(v) (1)
#define PAGER_INITIALIZED(p) (p->dbfh != NULL)


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
#define PAGE_TYPE_COUNT 11


/*****************************************************************
 * IO / Paging utility functions
 *****************************************************************/
static int read_page(FileHandle *fh, uint32_t pageno, uint32_t pagesize, uint32_t usablesize, uint8_t pageType, Page **pagep) {
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
    page->usableSize = usablesize;
    page->pageNo = pageno;
    page->pageType = pageType;
    page->dirty = 0;
    page->refCount = 0;
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
 * PageCache routines.
 *****************************************************************/
static inline int pagecache_create(PageCache *cache, uint32_t size) {
    cache->count = 0;
    return ptrmap_set_size(cache, size);
}

static inline int pagecache_has(PageCache *cache, uint32_t pageNo) {
    return ptrmap_has(cache, pageNo);
}

static inline Page* pagecache_get(PageCache *cache, uint32_t pageNo) {
    return (Page*)ptrmap_get_or(cache, pageNo, NULL);
}

static inline int pagecache_put(PageCache *cache, Page *page) {
    Page *existing = ptrmap_get_or(cache, page->pageNo, NULL);
    assert(existing == NULL);
    return ptrmap_set(cache, page->pageNo, page);
}

static inline uint32_t pagecache_count(PageCache *cache) {
    return cache->count;
}

static inline void pagecache_deinit(PageCache *cache) {
    ptrmap_deinit(cache);
}

static inline int pagecache_clear(PageCache *cache) {
    /* Free the pages */
    uint32_t index;
    ptrmap_entry* current;

    index = 0;
    if (!cache) {
        return FABRICDB_OK;
    }

    while(index < cache->size) {
        current = cache->items[index];
        while (current != NULL) {
            if (current->value) {
                free_page(current->value);
                current->value = NULL;
            }
            current = current->next;
        }
        index++;
    }

    return ptrmap_reinit(cache, cache->size);
}

/*****************************************************************
 * PageTypeCache routines.
 *****************************************************************/
static int pagetypecache_init(PageTypeCache *cache) {
    int rc = FABRICDB_OK;
    int i = 0;

    rc = u8array_set_size(&cache->allPages, 16);
    /* Do not use the first index since page numbers start with 1 */
    if (rc == FABRICDB_OK) {
        u8array_set(&cache->allPages, 0, 0);
    }
    while(i < PAGE_TYPE_COUNT && rc == FABRICDB_OK) {
        rc = u32array_set_size(&(cache->pageTypes[i]), 16);
        i++;
    }

    return rc;
}

static void pagetypecache_deinit(PageTypeCache *cache) {
    int i = 0;
    u8array_deinit(&cache->allPages);
    while(i < PAGE_TYPE_COUNT) {
        u32array_deinit(&(cache->pageTypes[i]));
        i++;
    }
}

static int pagetypecache_put(PageTypeCache* cache, uint32_t pageNo, uint8_t pageType) {
    u8array_set(&cache->allPages, pageNo, pageType);
    u32array_push(&cache->pageTypes[pageType], pageNo);
    cache->dirty = 1;

    return FABRICDB_OK;
}

static int pagetypecache_load(PageTypeCache* cache, Pager *pager, Page *page, uint32_t pageNo, off_t offset) {
    uint32_t pageSize;
    off_t beginOffset;
    uint8_t type;
    int rc = FABRICDB_OK;

    pageSize = page->pageSize;
    beginOffset = offset;

    while(offset < pageSize) {
        type = *(page->data + offset);
        pagetypecache_put(cache, pageNo, type);
        if (type == P_PAGE) {
            /* read the next page type page */
            assert(offset+1 == page->usableSize);
            rc = read_page(pager->dbfh, pageNo, pageSize, page->usableSize, type, &page);
            if (rc != FABRICDB_OK) {
                return rc;
            }
            rc = pagetypecache_load(cache, pager, page, pageNo+1, 0);
            free_page(page);
            break;
        } else if (type == UNUSED_PAGE) {
            /* we have loaded all the used pages */
            cache->dirty = 0;
            break;
        }
        offset++;
        pageNo++;
    }

    return rc;
}

static inline int pagetypecache_get_type(PageTypeCache *cache, uint32_t pageNo) {
    return u8array_get_or(&cache->allPages, pageNo, UNUSED_PAGE);
}



/*******************************************************************
 * Pager creation and initialization routines.
 *******************************************************************/

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
    pager->jfh = NULL;

    /* Defaults */
    pager->dbstate.fileChangeCounter = 0;
    pager->dbstate.filePageCount = 0;
    pager->dbstate.fileFreePageCount = 0;
    pager->dbstate.schemaCookie = 0;

    /* Defaults */
    pager->pragma.applicationId = 0;
    pager->pragma.applicationVersion = 0;
    pager->pragma.pageSize = FDB_DEFAULT_PAGE_SIZE;
    pager->pragma.fileFormatWriteVersion = 1;
    pager->pragma.fileFormatReadVersion = 1;
    pager->pragma.bytesReserved = 0;
    pager->pragma.defCacheSize = FDB_DEFAULT_CACHE_SIZE;
    pager->pragma.defAutoVacuum = 0;
    pager->pragma.defAutoVacuumThreshold = 0;
    pager->pragma.autoVacuum = 0;
    pager->pragma.autoVacuumThreshold = 0;
    pager->pragma.cacheSize = FDB_DEFAULT_CACHE_SIZE;

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
    rc = read_page(pager->dbfh, 1, page_size + num_reserved_bytes, page_size, HEADER_PAGE, &front_page);
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

    /* Initialize the cache */
    rc = pagecache_create(&pager->pageCache, pager->pragma.cacheSize);
    if (rc != FABRICDB_OK) {
        goto pager_init_done;
    }

    /* The rest of the front page (after the header) contains data
       that describes how every page is used.  After this data is
       read, every lookup by id in the database takes O(1) time
       because the exact position in the datafile can be calculated.
       The downside of this is that the page page has to be maintained. */
    rc = pagetypecache_init(&pager->pageTypeCache);
    if (rc != FABRICDB_OK) {
        goto pager_init_done;
    }
    rc = pagetypecache_load(&pager->pageTypeCache, pager, front_page,1,FDB_FILE_HEADER_SIZE);
    if (rc != FABRICDB_OK) {
        goto pager_init_done;
    }
    /* Ignore error code */
    pagecache_put(&pager->pageCache, front_page);

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
        pagecache_deinit(&pager->pageCache);
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

    /* set the first page to HEADER_PAGE */
    *(buffer + FDB_FILE_HEADER_SIZE) = HEADER_PAGE;

    /* Write to disk and sync */
    rc = fdb_write(pager->dbfh, buffer, 0, pager->pragma.pageSize + pager->pragma.bytesReserved);
    if (rc == FABRICDB_OK) {
        rc = fdb_sync(pager->dbfh);
    }

    fdbfree(buffer);
    if (rc != FABRICDB_OK) {
        fdb_close_file(pager->dbfh);
        pager->dbfh = NULL;
        return rc;
    }

    return fdb_pager_init_from_file(pager);
}

void fdb_pager_destroy(Pager *pager) {
    if(pager->filePath) {
        fdbfree(pager->filePath);
    }
    if (pager->dbfh) {
        fdb_close_file(pager->dbfh);
    }
    if (pager->jfh) {
        fdb_close_file(pager->jfh);
    }
    pagecache_clear(&pager->pageCache);
    pagecache_deinit(&pager->pageCache);
    pagetypecache_deinit(&pager->pageTypeCache);
    fdbfree(pager);
}

int fdb_pager_fetch_page(Pager *pager, uint32_t pageNo, Page** pagep) {
    int rc = FABRICDB_OK;
    int pageSize;
    uint8_t pageType;
    Page* page = pagecache_get(&pager->pageCache, pageNo);

    if (page != NULL) {
        *pagep = page;
        return rc;
    }

    /* Missed the cache so load it from disc */
    pageSize = pager->pragma.pageSize + pager->pragma.bytesReserved;
    pageType = pagetypecache_get_type(&pager->pageTypeCache, pageNo);
    rc = read_page(pager->dbfh, pageNo, pageSize, pager->pragma.pageSize, pageType, &page);
    if (pagecache_count(&pager->pageCache) >= pager->pragma.cacheSize) {
        /* TODO: need to clear the cache, but which to get rid of?? */
    }

    if (rc == FABRICDB_OK) {
        /* Add it to the cache */
        rc = pagecache_put(&pager->pageCache, page);
        if (rc != FABRICDB_OK) {
            fdbfree(page);
            page = NULL;
        }
    }

    *pagep = page;
    return rc;
}


/*******************************************************************
 * Pragma manipulation.
 *******************************************************************/
int fdb_pager_set_page_size(Pager *pager, uint32_t size) {
    if (PAGER_INITIALIZED(pager) || !VALID_PAGE_SIZE(size)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.pageSize = size;
    return FABRICDB_OK;
}

uint32_t fdb_pager_get_page_size(Pager *pager) {
    return pager->pragma.pageSize;
}

int fdb_pager_set_application_version(Pager *pager, uint32_t version) {
    if (PAGER_INITIALIZED(pager)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.applicationVersion = version;
    return FABRICDB_OK;
}

uint32_t fdb_pager_get_application_version(Pager *pager) {
    return pager->pragma.applicationVersion;
}

int fdb_pager_set_application_id(Pager *pager, uint32_t id) {
    if (PAGER_INITIALIZED(pager)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.applicationId = id;
    return FABRICDB_OK;
}

uint32_t fdb_pager_get_application_id(Pager *pager) {
    return pager->pragma.applicationId;
}

int fdb_pager_set_file_format_write_version(Pager *pager, uint8_t write_version) {
    if (PAGER_INITIALIZED(pager) || !VALID_FILE_FORMAT_WRITE_VERSION(write_version)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.fileFormatWriteVersion = write_version;
    return FABRICDB_OK;
}

uint8_t fdb_pager_get_file_format_write_version(Pager *pager) {
    return pager->pragma.fileFormatWriteVersion;
}

int fdb_pager_set_file_format_read_version(Pager *pager, uint8_t read_version) {
    if (PAGER_INITIALIZED(pager) || !VALID_FILE_FORMAT_READ_VERSION(read_version)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.fileFormatReadVersion = read_version;
    return FABRICDB_OK;
}

uint8_t fdb_pager_get_file_format_read_version(Pager *pager) {
    return pager->pragma.fileFormatReadVersion;
}

int fdb_pager_set_bytes_reserved_space(Pager *pager, uint8_t num_bytes) {
    if (PAGER_INITIALIZED(pager)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.bytesReserved = num_bytes;
    return FABRICDB_OK;
}

uint8_t fdb_pager_get_bytes_reserved_space(Pager *pager) {
    return pager->pragma.bytesReserved;
}

int fdb_pager_set_def_auto_vacuum(Pager *pager, uint8_t enabled) {
    if (PAGER_INITIALIZED(pager)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.defAutoVacuum = enabled;
    return FABRICDB_OK;
}

uint8_t fdb_pager_get_def_auto_vacuum(Pager *pager) {
    return pager->pragma.defAutoVacuum;
}

int fdb_pager_set_def_auto_vacuum_threshold(Pager *pager, uint8_t threshold) {
    if (PAGER_INITIALIZED(pager)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.defAutoVacuumThreshold = threshold;
    return FABRICDB_OK;
}

uint8_t fdb_pager_get_def_auto_vacuum_threshold(Pager *pager) {
    return pager->pragma.defAutoVacuumThreshold;
}

int fdb_pager_set_def_cache_size(Pager *pager, uint32_t num_pages) {
    if (PAGER_INITIALIZED(pager) || !VALID_CACHE_SIZE(num_pages)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.defCacheSize = num_pages;
    return FABRICDB_OK;
}

uint32_t fdb_pager_get_def_cache_size(Pager *pager) {
    return pager->pragma.defCacheSize;
}

int fdb_pager_set_auto_vacuum(Pager *pager, uint8_t enabled) {
    pager->pragma.autoVacuum = enabled;
    return FABRICDB_OK;
}

uint8_t fdb_pager_get_auto_vacuum(Pager *pager) {
    return pager->pragma.autoVacuum;
}

int fdb_pager_set_auto_vacuum_threshold(Pager *pager, uint8_t threshold) {
    pager->pragma.autoVacuumThreshold = threshold;
    return FABRICDB_OK;
}

uint8_t fdb_pager_get_auto_vacuum_threshold(Pager *pager) {
    return pager->pragma.autoVacuumThreshold;
}

int fdb_pager_set_cache_size(Pager *pager, uint32_t num_pages) {
    if (!VALID_CACHE_SIZE(num_pages)) {
        return FABRICDB_EMISUSE_PRAGMA;
    }

    pager->pragma.cacheSize = num_pages;
    return FABRICDB_OK;
}

uint32_t fdb_pager_get_cache_size(Pager *pager) {
    return pager->pragma.cacheSize;
}


 #ifdef FABRICDB_TESTING
 #include "../test/test_pager.c"
 #endif
