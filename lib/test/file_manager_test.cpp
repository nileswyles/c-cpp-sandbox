#include "tester.h"
#include "file/file.h"
#include "datastructures/array.h"
#include "paths.h"

#include <memory>

#ifndef LOGGER_FILE_MANAGER_TEST
#define LOGGER_FILE_MANAGER_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE_MANAGER_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;
using namespace WylesLibs::File;

static std::shared_ptr<FileManager> file_manager = std::make_shared<FileManager>();
static std::string test_directory = "./file_manager_test_dir";
static const std::string test_file = Paths::join(test_directory, "file_manager_test.txt");

void testFileManager(TestArg * t) {
    printf("?\n");
    uint64_t size = file_manager->stat(test_file);
    printf("lol?\n");
    if (size > 0) {
        printf("SIZE: %lu", size);
        t->fail = true;
        return;
    }

    SharedArray<uint8_t> file_data("Store some information in the file.");
    printf("lol2?\n");
    file_manager->write(test_file, file_data, false); // >
    printf("lol3?\n");
    SharedArray<uint8_t> read_file_data = file_manager->read(test_file);
    size = file_manager->stat(test_file);
    printf("lol4?\n");
    if (file_data != read_file_data || size != file_data.size()) {
        t->fail = true;
        return;
    }

    printf("lol5?\n");
    SharedArray<uint8_t> appended_file_data(" Append some information in the file.");
    file_manager->write(test_file, appended_file_data, true); // >>
    printf("lol6?\n");
    read_file_data = file_manager->read(test_file);
    if (file_data + appended_file_data != read_file_data) {
        t->fail = true;
        return;
    }

    SharedArray<uint8_t> overwritten_file_data("Store only this information in the file.");
    file_manager->write(test_file, overwritten_file_data, false); // >
    read_file_data = file_manager->read(test_file);
    if (overwritten_file_data != read_file_data) {
        t->fail = true;
        return;
    }

    SharedArray<std::string> directory_list = file_manager->list(test_directory);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Directory Listing:\n");
    for (auto item: directory_list) {
        size = file_manager->stat(Paths::join(test_directory, item));
        loggerPrintf(LOGGER_TEST_VERBOSE, "%s, %lu\n", item.c_str(), size);
    }

    file_manager->remove(test_file);
}

void beforeSuite() {
    system(("rm -r " + test_directory + " 2> /dev/null").c_str());
    system(("mkdir " + test_directory + " 2> /dev/null").c_str());
    system(("echo -n \"\" > " + test_file + " 2> /dev/null").c_str());
}

void afterSuite() {
    system(("rm -r " + test_directory + " 2> /dev/null").c_str());
}

int main(int argc, char * argv[]) {
    Tester t("File Manager Tests", beforeSuite, nullptr, afterSuite, nullptr);

    t.addTest(testFileManager);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}