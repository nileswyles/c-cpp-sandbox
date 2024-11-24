#include "tester.h"

#include "datastructures/array.h"
#include "file/file_watcher.h"
#include "paths.h"

#include <unistd.h>
#include <stdlib.h>
#include <memory>
#include "eshared_ptr.h"

#ifndef LOGGER_FILE_WATCHER_TEST
#define LOGGER_FILE_WATCHER_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE_WATCHER_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;

static void createFile(std::string path) {
    system(("echo somedata > " + path).c_str());
}

bool riskyCreateBool = false;
bool riskyMoveOutBool = false;
bool riskyMoveInBool = false;
bool riskyDeleteBool = false;

class TestFileWatcher: public FileWatcher {
    public:
        TestFileWatcher(SharedArray<std::string> paths): FileWatcher(paths, IN_CREATE | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE) {}
        void handle(const struct inotify_event * event) {
            loggerPrintf(LOGGER_DEBUG, "MASK: %x, NAME: %s\n", event->mask, event->name);
            if (event->mask&IN_CREATE) {
                loggerPrintf(LOGGER_DEBUG, "CREATE EVENT FOUND!\n");
                riskyCreateBool = true;
            } else if (event->mask&IN_MOVED_FROM) {
                loggerPrintf(LOGGER_DEBUG, "MOVE OUT EVENT FOUND!\n");
                riskyMoveOutBool = true;
            } else if (event->mask&IN_MOVED_TO) {
                loggerPrintf(LOGGER_DEBUG, "MOVE IN EVENT FOUND!\n");
                riskyMoveInBool = true;
            } else if (event->mask&IN_DELETE) {
                loggerPrintf(LOGGER_DEBUG, "DELETE EVENT FOUND!\n");
                riskyDeleteBool = true;
            }
        }
};

static std::string test_directory = "./file_watcher_test_dir";
static std::string test_directory_other = "./file_watcher_other_dir";
static TestFileWatcher * file_watcher;

static void testFileWatcherFileCreated(TestArg * t) {
    riskyCreateBool = false;
    std::string test_file = Paths::join(test_directory, "created_file");
    createFile(test_file);
    size_t i = 10;
    while (false == riskyCreateBool && i-- > 0) {
        sleep(1);
    }
    t->fail = false == riskyCreateBool;
}

static void testFileWatcherFileRemoved(TestArg * t) {
    riskyDeleteBool = false;
    std::string test_file = Paths::join(test_directory, "file_to_remove");
    createFile(test_file);
    system(("rm " + test_file + " 2> /dev/null").c_str());
    size_t i = 10;
    while (false == riskyDeleteBool && i-- > 0) {
        sleep(1);
    }
    t->fail = false == riskyDeleteBool;
}

static void testFileWatcherFileMovedIn(TestArg * t) {
    riskyMoveInBool = false;
    std::string test_file = Paths::join(test_directory_other, "moved_file");
    createFile(test_file);
    system(("mv " + test_file + " " + Paths::join(test_directory, "moved_file")).c_str());
    size_t i = 10;
    while (false == riskyMoveInBool && i-- > 0) {
        sleep(1);
    }
    t->fail = false == riskyMoveInBool;
}

static void testFileWatcherFileMovedOut(TestArg * t) {
    riskyMoveOutBool = false;
    std::string test_file = Paths::join(test_directory, "moved_file");
    createFile(test_file);
    system(("mv " + test_file + " " + Paths::join(test_directory_other, "moved_file")).c_str());
    size_t i = 10;
    while (false == riskyMoveOutBool && i-- > 0) {
        sleep(1);
    }
    t->fail = false == riskyMoveOutBool;
}

static void beforeSuite() {
    system(("rm -r " + test_directory + " 2> /dev/null").c_str());
    system(("rm -r " + test_directory_other + " 2> /dev/null").c_str());

    system(("mkdir " + test_directory + " 2> /dev/null").c_str());
    system(("mkdir " + test_directory_other + " 2> /dev/null").c_str());

    fileWatcherThreadStart();

    SharedArray<std::string> paths{test_directory};
    file_watcher = new TestFileWatcher(paths);
    file_watcher->initialize(file_watcher);
}

static void removeStoreFile() {
    fileWatcherThreadStop();
    // file_watcher = nullptr;

    system(("rm -r " + test_directory).c_str());
    system(("rm -r " + test_directory_other).c_str());
}

static void beforeEach(TestArg * t) {
    system(("rm " + Paths::join(test_directory, "/*") + " 2> /dev/null").c_str());
    system(("rm " + Paths::join(test_directory_other, "/*") + " 2> /dev/null").c_str());
}

int main(int argc, char * argv[]) {
    Tester t("File Watcher Tests", beforeSuite, beforeEach, removeStoreFile, nullptr);

    t.addTest(testFileWatcherFileCreated);
    // TODO: We really just want to confirm that we can register and receive events so the following tests aren't urgent.
    // t.addTest(testFileWatcherFileRemoved);
    // t.addTest(testFileWatcherFileMovedIn);
    // t.addTest(testFileWatcherFileMovedOut);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}