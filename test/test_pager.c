#include "test_common.h"

static const char* TEMPFILENAME = "./tempfile.tmp";

static const char* TESTSTRING = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
static const int TESTSTRING_SIZE= 36;

void test_read_page() {
    FileHandle *fh;
    Page* page;
    // uint8_t* buffer[TESTSTRING_SIZE];

    remove(TEMPFILENAME);

    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);
    fdb_assert("Could not create file", fdb_create_file(TEMPFILENAME, &fh) == FABRICDB_OK);

    fdb_assert("Lock level wrong", fdb_get_lock_level(fh) == FDB_NO_LOCK);
    fdb_assert("Could not write file", fdb_write(fh, (uint8_t*)TESTSTRING, 0, TESTSTRING_SIZE) == FABRICDB_OK);

    fdb_assert("Could not read page", read_page(fh, 1, TESTSTRING_SIZE, TESTSTRING_SIZE, 1, &page) == FABRICDB_OK);
    fdb_assert("Page was null", page);
    fdb_assert("Page size was wrong", page->pageSize == TESTSTRING_SIZE);
    fdb_assert("Page number was wrong", page->pageNo == 1);
    fdb_assert("Data is null", page->data);
    fdb_assert("Page type not correctly set", page->pageType == 1);
    fdb_assert("Page marked as dirty", page->dirty == 0);
    fdb_assert("Did not read correct values", memcmp(TESTSTRING, page->data, TESTSTRING_SIZE) == 0);

    free_page(page);
    fdb_assert("Lock level wrong", fdb_get_lock_level(fh) == FDB_NO_LOCK);
    fdb_close_file(fh);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_create_destroy_database() {
    Pager* pager;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    fdb_assert("Could not create pager", fdb_pager_create(TEMPFILENAME, &pager) == FABRICDB_OK);
    fdb_assert("Memory not allocated", fabricdb_mem_used() > 0);

    fdb_pager_destroy(pager);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_init_file() {
    Pager *pager;
    off_t fileSize;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    remove(TEMPFILENAME);

    fdb_assert("Could not create pager", fdb_pager_create(TEMPFILENAME, &pager) == FABRICDB_OK);
    fdb_assert("Init file failed", fdb_pager_init_file(pager) == FABRICDB_OK);


    fdb_assert("File change counter not 0", pager->dbstate.fileChangeCounter == 0);
    fdb_assert("File page count not 1", pager->dbstate.filePageCount == 1);
    fdb_assert("File free page count not 0", pager->dbstate.fileFreePageCount == 0);
    fdb_assert("File schema cookie not 0", pager->dbstate.schemaCookie == 0);

    fdb_assert("Application id not 0", pager->pragma.applicationId == 0);
    fdb_assert("Application version not 0", pager->pragma.applicationVersion == 0);
    fdb_assert("Page size not default", pager->pragma.pageSize == FDB_DEFAULT_PAGE_SIZE);
    fdb_assert("File format write version not 1", pager->pragma.fileFormatWriteVersion == 1);
    fdb_assert("File format read version not 1", pager->pragma.fileFormatReadVersion == 1);
    fdb_assert("Bytes reserved not 0", pager->pragma.bytesReserved == 0);
    fdb_assert("Default cache size not default", pager->pragma.defCacheSize == FDB_DEFAULT_CACHE_SIZE);
    fdb_assert("Def auto vacuum not 0", pager->pragma.defAutoVacuum == 0);
    fdb_assert("Def auto vacuum thresh not 0", pager->pragma.defAutoVacuumThreshold == 0);
    fdb_assert("Auto vacuum not 0", pager->pragma.autoVacuum == 0);
    fdb_assert("Def auto vacuum thresh not 0", pager->pragma.autoVacuumThreshold == 0);
    fdb_assert("Cache size not default", pager->pragma.cacheSize == FDB_DEFAULT_CACHE_SIZE);

    /* make sure full page was written */
    fdb_assert("Could not get file size", fdb_file_size(pager->dbfh, &fileSize) == FABRICDB_OK);
    fdb_assert("Not all bytes written", fileSize == pager->pragma.pageSize);

    /* check page cache */
    fdb_assert("Front page not set", pager->pageCache.count == 1);
    fdb_assert("Page cache does not have front page", pagecache_has(&pager->pageCache, 1) == 1);
    fdb_assert("Page cache does not have front page", pagecache_get(&pager->pageCache, 1) != NULL);
    fdb_assert("First page is not header page", pagecache_get(&pager->pageCache, 1)->pageType == HEADER_PAGE);
    fdb_assert("Page cache has second page", pagecache_has(&pager->pageCache, 2) == 0);
    fdb_assert("Page cache does not have front page", pagecache_get(&pager->pageCache, 2) == NULL);
    fdb_assert("Page cache has conflicting page", pagecache_has(&pager->pageCache, 1+pager->pageCache.size) == 0);

    /* check page type cache */
    fdb_assert("Page types not set", pager->pageTypeCache.allPages.data != NULL);
    fdb_assert("Page types not set", pager->pageTypeCache.allPages.count == 3);
    fdb_assert("Page types have no size", pager->pageTypeCache.allPages.size > 0);
    fdb_assert("First value isn't 0", u8array_get_or(&(pager->pageTypeCache.allPages), 0, 100) == 0);
    fdb_assert("Second value isn't 1", u8array_get_or(&(pager->pageTypeCache.allPages), 1, 0) == 1);
    fdb_assert("Third value is set", u8array_get_or(&(pager->pageTypeCache.allPages), 2, 0) == 0);
    fdb_assert("Front page not added to header pages", pager->pageTypeCache.pageTypes[HEADER_PAGE].count == 1);
    fdb_assert("Second page not added to unused pages", pager->pageTypeCache.pageTypes[UNUSED_PAGE].count == 1);
    fdb_assert("Had a vertex page", pager->pageTypeCache.pageTypes[VERTEX_PAGE].count == 0);

    fdb_pager_destroy(pager);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    /* Make sure we get the same results when just reading it */
    fdb_assert("Could not create pager", fdb_pager_create(TEMPFILENAME, &pager) == FABRICDB_OK);
    fdb_assert("Init file failed", fdb_pager_init(pager) == FABRICDB_OK);

    fdb_assert("File change counter not 0", pager->dbstate.fileChangeCounter == 0);
    fdb_assert("File page count not 1", pager->dbstate.filePageCount == 1);
    fdb_assert("File free page count not 0", pager->dbstate.fileFreePageCount == 0);
    fdb_assert("File schema cookie not 0", pager->dbstate.schemaCookie == 0);

    fdb_assert("Application id not 0", pager->pragma.applicationId == 0);
    fdb_assert("Application version not 0", pager->pragma.applicationVersion == 0);
    fdb_assert("Page size not default", pager->pragma.pageSize == FDB_DEFAULT_PAGE_SIZE);
    fdb_assert("File format write version not 1", pager->pragma.fileFormatWriteVersion == 1);
    fdb_assert("File format read version not 1", pager->pragma.fileFormatReadVersion == 1);
    fdb_assert("Bytes reserved not 0", pager->pragma.bytesReserved == 0);
    fdb_assert("Default cache size not default", pager->pragma.defCacheSize == FDB_DEFAULT_CACHE_SIZE);
    fdb_assert("Def auto vacuum not 0", pager->pragma.defAutoVacuum == 0);
    fdb_assert("Def auto vacuum thresh not 0", pager->pragma.defAutoVacuumThreshold == 0);
    fdb_assert("Auto vacuum not 0", pager->pragma.autoVacuum == 0);
    fdb_assert("Def auto vacuum thresh not 0", pager->pragma.autoVacuumThreshold == 0);
    fdb_assert("Cache size not default", pager->pragma.cacheSize == FDB_DEFAULT_CACHE_SIZE);

    fdb_pager_destroy(pager);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_init_file_2() {
    Pager *pager;
    off_t fileSize;
    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);

    remove(TEMPFILENAME);

    fdb_assert("Could not create pager", fdb_pager_create(TEMPFILENAME, &pager) == FABRICDB_OK);

    fdb_assert("Could not set page size", fdb_pager_set_page_size(pager, 2000) == FABRICDB_OK);
    fdb_assert("Set page size should fail", fdb_pager_set_page_size(pager, 10) == FABRICDB_EMISUSE_PRAGMA);
    fdb_assert("Could not get page size", fdb_pager_get_page_size(pager) == 2000);

    fdb_assert("Could not set application version", fdb_pager_set_application_version(pager, 12) == FABRICDB_OK);
    fdb_assert("Could not set application id", fdb_pager_set_application_id(pager, 24) == FABRICDB_OK);
    fdb_assert("Did not get correct version", fdb_pager_get_application_version(pager) == 12);
    fdb_assert("Did not get correct id", fdb_pager_get_application_id(pager) == 24);

    fdb_assert("Set write version didn't fail", fdb_pager_set_file_format_write_version(pager, 2) == FABRICDB_EMISUSE_PRAGMA);
    fdb_assert("Set read version didn't fail", fdb_pager_set_file_format_read_version(pager, 2) == FABRICDB_EMISUSE_PRAGMA);
    fdb_assert("Set write version failed", fdb_pager_set_file_format_write_version(pager, 1) == FABRICDB_OK);
    fdb_assert("Set read version failed", fdb_pager_set_file_format_read_version(pager, 1) == FABRICDB_OK);
    fdb_assert("Got wrong value for write version", fdb_pager_get_file_format_write_version(pager) == 1);
    fdb_assert("Got wrong value for read version", fdb_pager_get_file_format_read_version(pager) == 1);

    fdb_assert("Could not set reserved bytes", fdb_pager_set_bytes_reserved_space(pager, 12) == FABRICDB_OK);
    fdb_assert("Did not get correct reserved bytes", fdb_pager_get_bytes_reserved_space(pager) == 12);

    fdb_assert("Could not set def auto vacuum", fdb_pager_set_def_auto_vacuum(pager, 1) == FABRICDB_OK);
    fdb_assert("Could not set def auto vacuum thresh", fdb_pager_set_def_auto_vacuum_threshold(pager, 15) == FABRICDB_OK);
    fdb_assert("Did not get correct def auto vacuum", fdb_pager_get_def_auto_vacuum(pager) == 1);
    fdb_assert("Did not get correct def auto vacuum thresh", fdb_pager_get_def_auto_vacuum_threshold(pager) == 15);

    fdb_assert("Could not set def cache size", fdb_pager_set_def_cache_size(pager, 150) == FABRICDB_OK);
    fdb_assert("Did not get correct def cache size", fdb_pager_get_def_cache_size(pager) == 150);

    fdb_assert("Init file failed", fdb_pager_init_file(pager) == FABRICDB_OK);

    fdb_assert("File change counter not 0", pager->dbstate.fileChangeCounter == 0);
    fdb_assert("File page count not 1", pager->dbstate.filePageCount == 1);
    fdb_assert("File free page count not 0", pager->dbstate.fileFreePageCount == 0);
    fdb_assert("File schema cookie not 0", pager->dbstate.schemaCookie == 0);

    fdb_assert("Application id not 0", pager->pragma.applicationId == 24);
    fdb_assert("Application version not 0", pager->pragma.applicationVersion == 12);
    fdb_assert("Page size not default", pager->pragma.pageSize == 2000);
    fdb_assert("File format write version not 1", pager->pragma.fileFormatWriteVersion == 1);
    fdb_assert("File format read version not 1", pager->pragma.fileFormatReadVersion == 1);
    fdb_assert("Bytes reserved not 0", pager->pragma.bytesReserved == 12);
    fdb_assert("Default cache size not default", pager->pragma.defCacheSize == 150);
    fdb_assert("Def auto vacuum not 0", pager->pragma.defAutoVacuum == 1);
    fdb_assert("Def auto vacuum thresh not 0", pager->pragma.defAutoVacuumThreshold == 15);
    fdb_assert("Auto vacuum not 0", pager->pragma.autoVacuum == 1);
    fdb_assert("Def auto vacuum thresh not 0", pager->pragma.autoVacuumThreshold == 15);
    fdb_assert("Cache size not default", pager->pragma.cacheSize == 150);

    /* make sure full page was written */
    fdb_assert("Could not get file size", fdb_file_size(pager->dbfh, &fileSize) == FABRICDB_OK);
    fdb_assert("Not all bytes written", fileSize == pager->pragma.pageSize + 12);

    fdb_assert("Could set page size", fdb_pager_set_page_size(pager, 2000) != FABRICDB_OK);
    fdb_assert("Could set application version", fdb_pager_set_application_version(pager, 12) != FABRICDB_OK);
    fdb_assert("Could set application id", fdb_pager_set_application_id(pager, 24) != FABRICDB_OK);
    fdb_assert("Could set reserved bytes", fdb_pager_set_bytes_reserved_space(pager, 12) != FABRICDB_OK);
    fdb_assert("Could set def auto vacuum", fdb_pager_set_def_auto_vacuum(pager, 1) != FABRICDB_OK);
    fdb_assert("Could set def auto vacuum thresh", fdb_pager_set_def_auto_vacuum_threshold(pager, 15) != FABRICDB_OK);
    fdb_assert("Could set def cache size", fdb_pager_set_def_cache_size(pager, 150) != FABRICDB_OK);

    fdb_pager_destroy(pager);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    /* Make sure we get the same results when just reading it */
    fdb_assert("Could not create pager", fdb_pager_create(TEMPFILENAME, &pager) == FABRICDB_OK);
    fdb_assert("Init file failed", fdb_pager_init(pager) == FABRICDB_OK);

    fdb_assert("File change counter not 0", pager->dbstate.fileChangeCounter == 0);
    fdb_assert("File page count not 1", pager->dbstate.filePageCount == 1);
    fdb_assert("File free page count not 0", pager->dbstate.fileFreePageCount == 0);
    fdb_assert("File schema cookie not 0", pager->dbstate.schemaCookie == 0);

    fdb_assert("Application id not 0", pager->pragma.applicationId == 24);
    fdb_assert("Application version not 0", pager->pragma.applicationVersion == 12);
    fdb_assert("Page size not default", pager->pragma.pageSize == 2000);
    fdb_assert("File format write version not 1", pager->pragma.fileFormatWriteVersion == 1);
    fdb_assert("File format read version not 1", pager->pragma.fileFormatReadVersion == 1);
    fdb_assert("Bytes reserved not 0", pager->pragma.bytesReserved == 12);
    fdb_assert("Default cache size not default", pager->pragma.defCacheSize == 150);
    fdb_assert("Def auto vacuum not 0", pager->pragma.defAutoVacuum == 1);
    fdb_assert("Def auto vacuum thresh not 0", pager->pragma.defAutoVacuumThreshold == 15);
    fdb_assert("Auto vacuum not 0", pager->pragma.autoVacuum == 1);
    fdb_assert("Def auto vacuum thresh not 0", pager->pragma.autoVacuumThreshold == 15);
    fdb_assert("Cache size not default", pager->pragma.cacheSize == 150);

    fdb_assert("Could set page size", fdb_pager_set_page_size(pager, 2000) != FABRICDB_OK);
    fdb_assert("Could set application version", fdb_pager_set_application_version(pager, 12) != FABRICDB_OK);
    fdb_assert("Could set application id", fdb_pager_set_application_id(pager, 24) != FABRICDB_OK);
    fdb_assert("Could set reserved bytes", fdb_pager_set_bytes_reserved_space(pager, 12) != FABRICDB_OK);
    fdb_assert("Could set def auto vacuum", fdb_pager_set_def_auto_vacuum(pager, 1) != FABRICDB_OK);
    fdb_assert("Could set def auto vacuum thresh", fdb_pager_set_def_auto_vacuum_threshold(pager, 15) != FABRICDB_OK);
    fdb_assert("Could set def cache size", fdb_pager_set_def_cache_size(pager, 150) != FABRICDB_OK);

    fdb_assert("Could not set auto vacuum", fdb_pager_set_auto_vacuum(pager, 0) == FABRICDB_OK);
    fdb_assert("Could not set auto vacuum threshold", fdb_pager_set_auto_vacuum_threshold(pager, 2) == FABRICDB_OK);
    fdb_assert("Could not set cache size", fdb_pager_set_cache_size(pager, 10) == FABRICDB_OK);
    fdb_assert("Did not get correct value for auto vacuum", fdb_pager_get_auto_vacuum(pager) == 0);
    fdb_assert("Did not get correct value for auto vacuum thresh", fdb_pager_get_auto_vacuum_threshold(pager) == 2);
    fdb_assert("Did not get correct value for cache size", fdb_pager_get_cache_size(pager) == 10);


    fdb_pager_destroy(pager);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_pager() {
    fdb_runtest("Read page", test_read_page);
    fdb_runtest("Create / Destroy database", test_create_destroy_database);
    fdb_runtest("Init file", test_init_file);
    fdb_runtest("Init file 2", test_init_file_2);
}
