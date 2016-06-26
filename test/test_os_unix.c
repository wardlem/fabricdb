#include "test_common.h"

static const char* TEMPFILENAME = "./tempfile.tmp";
static const char* TEMPFILENAME_2 = "./tempfile2.tmp";
static const char* TEMPFILENAME_3 = "./tempfile3.tmp";
static const char* FAKEFILENAME = "./fakefile.tmp";

void test_create_file() {
    FileHandle *fh;
    int rc;

    remove(TEMPFILENAME);

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);

    rc = fdb_create_file(TEMPFILENAME, &fh);
    fdb_assert("Opening the file was not successful", FABRICDB_OK == rc);

    fdb_assert("Returned null", fh);

    fdb_assert("Did not open file descriptor", fh->fd >= MIN_FILE_DESCRIPTOR);
    fdb_assert("Did not copy file path", TEMPFILENAME != fh->filePath);
    fdb_assert("Did not set file path", fh->filePath);
    fdb_assert("Lock level not zeroed", fh->lockLevel == 0);
    fdb_assert("Inode Info not set", fh->inodeInfo);
    fdb_assert("Did not track mem usage", fabricdb_mem_used() > 0);

    fdb_assert("Closing file failed", !fdb_close_file(fh));

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    rc = fdb_create_file(TEMPFILENAME, &fh);
    fdb_assert("File was overwritten", FABRICDB_OK != rc);
    fdb_assert("Did not return null pointer", fh == NULL);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_open_file_rdwr() {
    FileHandle *fh;
    int rc;

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);
    rc = fdb_open_file_rdwr(TEMPFILENAME, &fh);
    fdb_assert("Opening the file was not successful", FABRICDB_OK == rc);
    fdb_assert("Did not open file descriptor", fh->fd >= MIN_FILE_DESCRIPTOR);
    fdb_assert("Did not copy file path", TEMPFILENAME != fh->filePath);
    fdb_assert("Did not set file path", fh->filePath);
    fdb_assert("Lock level not zeroed", fh->lockLevel == 0);
    fdb_assert("Inode Info not set", fh->inodeInfo);
    fdb_assert("Did not track mem usage", fabricdb_mem_used() > 0);

    fdb_assert("Closing file failed", !fdb_close_file(fh));
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    rc = fdb_open_file_rdwr(FAKEFILENAME, &fh);
    fdb_assert("Didn't fail when it should have", FABRICDB_OK != rc);
    fdb_assert("Did not return null pointer", fh == NULL);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_open_file_rdonly() {
    FileHandle *fh;
    int rc;

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);
    rc = fdb_open_file_rdonly(TEMPFILENAME, &fh);
    fdb_assert("Opening the file was not successful", FABRICDB_OK == rc);
    fdb_assert("Did not open file descriptor", fh->fd >= MIN_FILE_DESCRIPTOR);
    fdb_assert("Did not copy file path", TEMPFILENAME != fh->filePath);
    fdb_assert("Did not set file path", fh->filePath);
    fdb_assert("Lock level not zeroed", fh->lockLevel == 0);
    fdb_assert("Inode Info not set", fh->inodeInfo);
    fdb_assert("Did not track mem usage", fabricdb_mem_used() > 0);

    fdb_assert("Closing file failed", !fdb_close_file(fh));
    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    rc = fdb_open_file_rdonly(FAKEFILENAME, &fh);
    fdb_assert("Didn't fail when it should have", FABRICDB_OK != rc);
    fdb_assert("Did not return null pointer", fh == NULL);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_inodeinfo_fetch() {
    FileHandle *fh1;
    FileHandle *fh2;
    FileHandle *fh3;
    FileHandle *fh4;

    remove(TEMPFILENAME_2);
    remove(TEMPFILENAME_3);

    fdb_assert("Started test with memory used", fabricdb_mem_used() == 0);
    fdb_assert("Inode info list is not null", inodeInfoList == NULL);

    fdb_assert("Could not open file", 0 == fdb_open_file_rdwr(TEMPFILENAME, &fh1));
    fdb_assert("Null file handle", fh1);
    fdb_assert("Inode not set", fh1->inodeInfo);
    fdb_assert("Reference count not set", fh1->inodeInfo->refCount == 1);
    fdb_assert("Inode info list not set", inodeInfoList == fh1->inodeInfo);
    fdb_assert("Inode prev is not null", inodeInfoList->next == NULL);
    fdb_assert("Inode next not set", inodeInfoList->next == NULL);

    fdb_assert("Could not open file", 0 == fdb_open_file_rdwr(TEMPFILENAME, &fh2));

    fdb_assert("Null file handle", fh2);
    fdb_assert("Inode not set", fh2->inodeInfo);
    fdb_assert("Reference count not updated", fh2->inodeInfo->refCount == 2);
    fdb_assert("Inode prev is not null", inodeInfoList->next == NULL);
    fdb_assert("Inode next not set", inodeInfoList->next == NULL);

    fdb_assert("Have same file descriptor", fh1->fd != fh2->fd);
    fdb_assert("Have different inode infos", fh1->inodeInfo == fh2->inodeInfo);
    fdb_assert("Has a non null unused files", fh1->inodeInfo->unusedFiles == NULL);

    fdb_assert("Could not open file", 0 == fdb_create_file(TEMPFILENAME_2, &fh3));
    fdb_assert("Null file handle", fh3);
    fdb_assert("Inode not set", fh3->inodeInfo);
    fdb_assert("Reference count not updated", fh2->inodeInfo->refCount == 2);
    fdb_assert("Reference count not updated", fh3->inodeInfo->refCount == 1);

    fdb_assert("Inode list not updated", inodeInfoList == fh3->inodeInfo);
    fdb_assert("Inode prev is not null", inodeInfoList->prev == NULL);
    fdb_assert("Inode next not set", inodeInfoList->next == fh2->inodeInfo);
    fdb_assert("Inode prev not set", inodeInfoList->next->prev == inodeInfoList);
    fdb_assert("Inode next set", inodeInfoList->next->next == NULL);

    fdb_open_file_rdwr(TEMPFILENAME, &fh4);
    fdb_assert("Inode not found correctly", fh4->inodeInfo == fh1->inodeInfo);
    fdb_assert("Inode refCount not incremented", fh4->inodeInfo->refCount == 3);

    fdb_close_file(fh4);

    fdb_close_file(fh3);
    fdb_assert("Inode list not updated", inodeInfoList == fh1->inodeInfo);
    fdb_assert("Inode prev is not null", inodeInfoList->prev == NULL);
    fdb_assert("Inode next is not null", inodeInfoList->next == NULL);

    fdb_close_file(fh1);
    // no locks yet
    fdb_assert("Has a non null unused files", fh2->inodeInfo->unusedFiles == NULL);

    fdb_assert("Reference count not decremented", fh2->inodeInfo->refCount == 1);
    fdb_close_file(fh2);

    fdb_assert("Did not clean up all the memory", fabricdb_mem_used() == 0);

    fdb_passed;
}

void test_close_file() {

    fdb_passed;
}

void test_os_unix() {
    fdb_runtest("Create File", test_create_file);
    fdb_runtest("Open File Read-Write", test_open_file_rdwr);
    fdb_runtest("Open File Read Only", test_open_file_rdonly);
    fdb_runtest("Fetch Inode Info", test_inodeinfo_fetch);
    fdb_runtest("Close File", test_close_file);
}
