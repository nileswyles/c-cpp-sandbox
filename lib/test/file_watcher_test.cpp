#include "tester.h"

#include "array.h"
#include "file_watcher.h"
#include "paths.h"

#include <unistd.h>
#include <stdlib.h>
#include <memory>

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
bool riskyMoveBool = false;
bool riskyDeleteBool = false;

class TestFileWatcher: public FileWatcher {
    public:
        TestFileWatcher(Array<std::string> paths): FileWatcher(paths, IN_CREATE | IN_MOVE | IN_DELETE) {}
        void handle(const struct inotify_event * event) {
            if (event->mask == IN_CREATE) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "CREATE EVENT FOUND!\n");
                riskyCreateBool = true;
            } else if (event->mask == IN_MOVE) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "MOVE EVENT FOUND!\n");
                riskyMoveBool = true;
            } else if (event->mask == IN_DELETE) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "DELETE EVENT FOUND!\n");
                riskyDeleteBool = true;
            }
        }
};

static std::string test_directory = "./file_watcher_test_dir";
static std::string test_directory_other = "./file_watcher_other_dir";
static std::shared_ptr<TestFileWatcher> file_watcher;

// typedef void(DynamicHandle_t *)(const struct inotify_event * event);
// static DynamicHandle

static void testFileWatcherFileCreated(TestArg * t) {
    riskyCreateBool = false;
    std::string test_create_file = Paths::join(test_directory, "created_file");
    createFile(test_create_file);
    size_t i = 10;
    // lol, this 
    while (!riskyCreateBool && i-- > 0) {
        sleep(1);
    }
    if (riskyCreateBool) {
        t->fail = false;
    }
}

static void testFileWatcherFileRemoved(TestArg * t) {
    riskyDeleteBool = false;
    std::string test_file = Paths::join(test_directory, "file_to_remove");
    createFile(test_file);
    sleep(10);
    system(("rm " + test_file).c_str());
    size_t i = 10;
    while (!riskyDeleteBool && i-- > 0) {
        sleep(1);
    }
    if (riskyDeleteBool) {
        t->fail = false;
    }
}

static void testFileWatcherFileMovedIn(TestArg * t) {
    riskyMoveBool = false;
    std::string test_file = Paths::join(test_directory_other, "moved_file");
    createFile(test_file);
    system(("mv " + test_file + " " + Paths::join(test_directory, "moved_file")).c_str());
    size_t i = 10;
    while (!riskyMoveBool && i-- > 0) {
        sleep(1);
    }
    if (riskyMoveBool) {
        t->fail = false;
    }
}

static void testFileWatcherFileMovedOut(TestArg * t) {
    riskyMoveBool = false;
    std::string test_file = Paths::join(test_directory, "moved_file");
    createFile(test_file);
    system(("mv " + test_file + " " + Paths::join(test_directory_other, "moved_file")).c_str());
    size_t i = 10;
    while (!riskyMoveBool && i-- > 0) {
        sleep(1);
    }
    if (riskyMoveBool) {
        t->fail = false;
    }
}

static void beforeSuite() {
    system(("rm -r " + test_directory + " 2> /dev/null").c_str());
    system(("rm -r " + test_directory_other + " 2> /dev/null").c_str());

    system(("mkdir " + test_directory + " 2> /dev/null").c_str());
    system(("mkdir " + test_directory_other + " 2> /dev/null").c_str());

    fileWatcherThreadStart();

    // would be lame if I need to enumerate and list all files? that would defeat purpose?
    //  maybe just directories?
    Array<std::string> paths{test_directory};
    // file_watcher = std::make_shared<TestFileWatcher>({test_directory});
    file_watcher = std::make_shared<TestFileWatcher>(paths);
    file_watcher->initialize(file_watcher);
}

static void afterSuite() {
    printf("AFTER CALLED\n");
    // TODO: cleanup, if needed
    fileWatcherThreadStop();

    system(("rm -r " + test_directory).c_str());
    system(("rm -r " + test_directory_other).c_str());
}

int main(int argc, char * argv[]) {
    Tester t("Key Generator Tests", beforeSuite, nullptr, afterSuite, nullptr);

    t.addTest(testFileWatcherFileCreated);
    t.addTest(testFileWatcherFileRemoved);
    t.addTest(testFileWatcherFileMovedIn);
    t.addTest(testFileWatcherFileMovedOut);

    // TODO: catch exceptions so that before and after suite is still performed?
    //      Where too? in run function?
    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}