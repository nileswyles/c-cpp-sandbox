#include "tester.h"
#include "key_generator.h"

#ifndef LOGGER_KEY_GENERATOR_TEST
#define LOGGER_KEY_GENERATOR_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_KEY_GENERATOR_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;

void testUniqueKeyGenerator(TestArg * t) {
    ServerConfig config;
    printf("Test\n");
    UniqueKeyGenerator generator(config, UniqueKeyGeneratorStore("sequence_store"));
    printf("Test2\n");

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
    UniqueKeyGenerator generator(config, UniqueKeyGeneratorStore("sequence_store_string"));

    Array<std::string> expected_keys{
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
        loggerPrintf(LOGGER_TEST, "expected key: %s\n", expected_keys.buf[0].c_str());
        if (key != expected_keys.buf[0]) {
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
    // Tester t;

    // t.addTest(testUniqueKeyGenerator);
    // t.addTest(testUUIDGeneratorV4);
    // t.addTest(testUUIDGeneratorV7);

    TestArg t;
    testUniqueKeyGenerator(&t);
    testUUIDGeneratorV4(&t);
    testUUIDGeneratorV7(&t);

    // if (argc > 1) {
    //     loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
    //     t.run(argv[1]);
    // } else {
    //     t.run(nullptr);
    // }

    return 0;
}