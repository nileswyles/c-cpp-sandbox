#include "tester.h"
#ifdef FILE_MANAGER_GCS_TEST
#include "file/file_gcs.h"
#include "google/cloud/log.h"
#else
#include "file/file.h"
#endif
#include "datastructures/array.h"
#include "paths.h"

#include <iostream>

#include <memory>
#include "memory/pointers.h"

#ifndef LOGGER_FILE_MANAGER_TEST
#define LOGGER_FILE_MANAGER_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE_MANAGER_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;
using namespace WylesLibs::File;

#ifdef FILE_MANAGER_GCS_TEST
static FileManager * file_manager = dynamic_cast<FileManager *>(new GCSFileManager("test-bucket-free-tier"));
#else
static FileManager * file_manager = new FileManager();
#endif 
static std::string test_directory = "./file_manager_test_dir";
static const std::string test_file = Paths::join(test_directory, "file_manager_test.txt");
static const std::string test_file_doesnt_exist = Paths::join(test_directory, "file_manager_test_doesnt_exist.txt");
static uint64_t INITIAL_FILE_SIZE = 0;


bool assertFileData(SharedArray<uint8_t> expected, SharedArray<uint8_t> actual) {
    bool res = false;
    loggerPrintf(LOGGER_TEST, "Expected File Data (%lu):\n'%s'\n", expected.size(), expected.toString().c_str());
    loggerPrintf(LOGGER_TEST, "Actual File Data (%lu):\n'%s'\n", actual.size(), actual.toString().c_str());
    if (expected == actual) {
        res = true;
    }
    return res;
}

bool assertDirectory(SharedArray<std::string> directory_list) {
    bool res = true;
    loggerPrintf(LOGGER_TEST, "Directory Listing:\n");
    for (auto item: directory_list) {
        // size = file_manager->stat(Paths::join(test_directory, item));
        uint64_t size = file_manager->stat(item);
        loggerPrintf(LOGGER_TEST, "%s, %lu\n", item.c_str(), size);
        if (size != INITIAL_FILE_SIZE) {
            res = false;
        }
    }
    return res;
}

void testFileManager(TestArg * t) {
    SharedArray<uint8_t> file_data("Store some information in the file.");
    file_manager->write(test_file, file_data, false); // >
    SharedArray<uint8_t> read_file_data = file_manager->read(test_file);
    if (false == assertFileData(file_data, read_file_data)) {
        t->fail = true;
        return;
    }

    // SharedArray<uint8_t> appended_file_data(" Append some information in the file.");
    // file_manager->write(test_file, appended_file_data, true); // >>
    // read_file_data = file_manager->read(test_file);
    // // TODO: char specialization for append?
    // // file_data.removeBack(); // remove NUL char.
    // file_data.remove(file_data.size()-2, 2); // hmm....
    // file_data.append(appended_file_data);
    // if (false == assertFileData(file_data, read_file_data)) {
    //     t->fail = true;
    //     return;
    // }

    SharedArray<uint8_t> overwritten_file_data("Store only this information in the file.");
    file_manager->write(test_file, overwritten_file_data, false); // >
    read_file_data = file_manager->read(test_file);
    if (false == assertFileData(overwritten_file_data, read_file_data)) {
        t->fail = true;
    } else {
        t->fail = false;
    }
}
void testFileManagerWriteFileDoesntExist(TestArg * t) {
    SharedArray<uint8_t> file_data("Store some information in the file.");
    file_manager->write(test_file_doesnt_exist, file_data, false); // >
    SharedArray<uint8_t> read_file_data = file_manager->read(test_file_doesnt_exist);
    if (false == assertFileData(file_data, read_file_data)) {
        t->fail = true;
    } else {
        t->fail = false;
    }
}
void testFileManagerReadFileDoesntExist(TestArg * t) {
    bool exception = false;
    try {
        file_manager->read(test_file_doesnt_exist);
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Exception: %s\n", e.what());
        exception = true;
    }
    ASSERT_BOOLEAN(t, exception, true);
}

// ! IMPORTANT - the file_manager functions are expected to throw an exception if the file does not exist. 
// TODO: Write tests for this functionality eventually
void testFileManagerStat(TestArg * t) {
    // just to exercise stat func, i guess
    uint64_t size = file_manager->stat(test_file);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Initial Size: %lu\n", size);
    if (size != INITIAL_FILE_SIZE) {
        t->fail = true;
    } else {
        t->fail = false;
    }
}
void testFileManagerDirectoryListing(TestArg * t) {
    SharedArray<std::string> directory_list = file_manager->list(test_directory);
    if (false == assertDirectory(directory_list) || directory_list.size() != 1 || false == directory_list.contains(test_file)) {
        t->fail = true;
    } else {
        t->fail = false;
    }
}
void testFileManagerCopy(TestArg * t) {
    std::string copy_file = Paths::join(test_directory, "file_manager_test_copy.txt");
    file_manager->copy(test_file, copy_file);
    SharedArray<std::string> directory_list = file_manager->list(test_directory);
    if (false == assertDirectory(directory_list) || directory_list.size() != 2 || false == directory_list.contains(copy_file)) {
        t->fail = true;
    } else {
        t->fail = false;
    }
}
void testFileManagerMove(TestArg * t) {
    std::string move_file = Paths::join(test_directory, "file_manager_test_move.txt");
    file_manager->move(test_file, move_file);
    SharedArray<std::string> directory_list = file_manager->list(test_directory);
    if (false == assertDirectory(directory_list) || directory_list.size() != 1 || false == directory_list.contains(move_file)) {
        t->fail = true;
    } else {
        t->fail = false;
    }
}
void testFileManagerRemove(TestArg * t) {
    file_manager->remove(test_file);
    SharedArray<std::string> directory_list = file_manager->list(test_directory);
    if (false == assertDirectory(directory_list) || directory_list.size() != 0) {
        t->fail = true;
    } else {
        t->fail = false;
    }
}

void beforeSuite() {
    system(("rm -r " + test_directory + " 2> /dev/null").c_str());
    system(("mkdir " + test_directory + " 2> /dev/null").c_str());
    system(("echo -n \"\" > " + test_file + " 2> /dev/null").c_str());
}
void afterSuite() {
    system(("rm -r " + test_directory + " 2> /dev/null").c_str());
}
void beforeEach(TestArg * t) {
    std::string initial_file_contents("E");
    INITIAL_FILE_SIZE = initial_file_contents.size();
    system(("echo -n \"" + initial_file_contents + "\" > " + test_file + " 2> /dev/null").c_str());
}
void afterEach(TestArg * t) {
    system(("rm -f " + Paths::join(test_directory, "/*") + " 2> /dev/null").c_str());
}

int main(int argc, char * argv[]) {
    Tester t("File Manager Tests", beforeSuite, beforeEach, afterSuite, afterEach);

    t.addTest(testFileManager);
    t.addTest(testFileManagerWriteFileDoesntExist);
    t.addTest(testFileManagerReadFileDoesntExist);
    t.addTest(testFileManagerStat);
    t.addTest(testFileManagerDirectoryListing);
    t.addTest(testFileManagerCopy);
    t.addTest(testFileManagerMove);
    // t.addTest(testFileManagerRemove);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}