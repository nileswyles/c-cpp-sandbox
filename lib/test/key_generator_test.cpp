#include "tester.h"
#include "key_generator.h"

#include <memory>

#ifndef LOGGER_KEY_GENERATOR_TEST
#define LOGGER_KEY_GENERATOR_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_KEY_GENERATOR_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;
using namespace WylesLibs::File;

static std::shared_ptr<FileManager> file_manager = std::make_shared<FileManager>();

static void beforeEach(TestArg * t) {
    File::write("sequence_store", std::string("0000000000000000"), false); // clear file store
}

void testUniqueKeyGenerator(TestArg * t) {
    ServerConfig config;
    UniqueKeyGenerator generator(config, UniqueKeyGeneratorStore(file_manager, "sequence_store"));

    bool failed = false;
    for (size_t i = 0; i < 7; i++) {
        uint64_t key = generator.nextAsInteger();
        loggerPrintf(LOGGER_TEST, "key: %lu\n", key);
        loggerPrintf(LOGGER_TEST, "expected key: %lu\n", i + 1);
        if (key != i + 1) {
            failed = true;
        }
    }
    t->fail = failed;
}

void testUniqueKeyStringGenerator(TestArg * t) {
    ServerConfig config;
    UniqueKeyGenerator generator(config, UniqueKeyGeneratorStore(file_manager, "sequence_store"));

    SharedArray<std::string> expected_keys{
        "0000000000000000",
        "0000000000000001",
        "0000000000000002",
        "0000000000000003",
        "0000000000000004",
        "0000000000000005",
        "0000000000000006",
        "0000000000000007"
        "0000000000000008",
        "0000000000000009",
        "000000000000000A",
        "000000000000000B",
        "000000000000000C",
        "000000000000000D",
        "000000000000000F",
        "0000000000000010"
    };

    bool failed = false;
    for (size_t i = 0; i < 7; i++) {
        std::string key = generator.next();
        loggerPrintf(LOGGER_TEST, "key: %s\n", key.c_str());
        loggerPrintf(LOGGER_TEST, "expected key: %s\n", expected_keys[0].c_str());
        if (key != expected_keys[0]) {
            failed = true;
        }
    }
    t->fail = failed;
}

void testUUIDGeneratorV4(TestArg * t) {
    ServerConfig config;
    UUIDGeneratorV4 generator;

    bool failed = false;
    for (size_t i = 0; i < 7; i++) {
        std::string key = generator.next();
        loggerPrintf(LOGGER_TEST, "key: %s\n", key.c_str());
        if (key.size() != 32) {
            failed = true;
            // TODO: test random function? for randomness lol?
            //      maybe have applications run test at startup?
            //      lol, seems a bit much...
            //      
            //      and by randomness test, I mean detect any patterns, regularities?
            //       
        }
    }
    t->fail = failed;
}

void testUUIDGeneratorV7(TestArg * t) {
    ServerConfig config;
    UUIDGeneratorV7 generator;

    bool failed = false;
    for (size_t i = 0; i < 7; i++) {
        std::string key = generator.next();
        loggerPrintf(LOGGER_TEST, "key: %s\n", key.c_str());
        if (key.size() != 40) {
            failed = true;
        }
    }
    t->fail = failed;
}

int main(int argc, char * argv[]) {
    Tester t("Key Generator Tests", nullptr, beforeEach, nullptr, nullptr);

    t.addTest(testUniqueKeyGenerator);
    t.addTest(testUUIDGeneratorV4);
    t.addTest(testUUIDGeneratorV7);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}