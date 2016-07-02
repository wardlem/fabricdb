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
    fdb_assert("Could not write file", fdb_write(fh, (uint8_t*)TESTSTRING, 0, TESTSTRING_SIZE) == FABRICDB_OK);

    fdb_assert("Could not read page", read_page(fh, 1, TESTSTRING_SIZE, 1, &page) == FABRICDB_OK);
    fdb_assert("Page was null", page);
    fdb_assert("Page size was wrong", page->pageSize == TESTSTRING_SIZE);
    fdb_assert("Page number was wrong", page->pageNo == 1);
    fdb_assert("Data is null", page->data);
    fdb_assert("Page type not correctly set", page->pageType == 1);
    fdb_assert("Page marked as dirty", page->dirty == 0);
    fdb_assert("Did not read correct values", memcmp(TESTSTRING, page->data, TESTSTRING_SIZE) == 0);

    free_page(page);
    fdb_close_file(fh);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_write_page() {
    FileHandle *fh;
    Page* page;
    uint8_t* buffer[TESTSTRING_SIZE];

    remove(TEMPFILENAME);

    fdb_assert("Started with unclean memory", fabricdb_mem_used() == 0);
    fdb_assert("Could not create file", fdb_create_file(TEMPFILENAME, &fh) == FABRICDB_OK);


    // free_page(page);
    fdb_close_file(fh);
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);
    fdb_passed;
}

void test_pager() {
    fdb_runtest("Read page", test_read_page);
}
