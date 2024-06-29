#include "array.h"
#include "tester.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <string>
#include <stdio.h>

#ifndef LOGGER_ARRAY_TEST
#define LOGGER_ARRAY_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ARRAY_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;

template<typename T>
bool assert(Array<T> actual, T * expected, size_t expected_size, size_t expected_cap) {
    bool memory_match = memcmp((void *)expected, (void *)actual.buf(), expected_size * sizeof(T)) == 0;
    bool size_match = actual.size() == expected_size;
    // make sure cap grows at the predetermined rate.
    // also make sure size doesn't exceed cap (arguably more important).

    bool cap_match = actual.cap() == expected_cap;
    loggerPrintf(LOGGER_TEST, "Expected:\n");
    loggerPrintByteArray(LOGGER_TEST, (uint8_t *)expected, expected_size * sizeof(T));
    loggerPrintf(LOGGER_TEST, "Actual:\n");
    loggerPrintByteArray(LOGGER_TEST, (uint8_t *)actual.buf(), actual.size() * sizeof(T));
    loggerPrintf(LOGGER_TEST, "Memory Match: %s, Size Match: %s (expected: %lu, actual: %lu), Cap Match: %s (expected: %lu, actual: %lu)\n", 
        memory_match ? "True" : "False", 
        size_match ? "True" : "False", expected_size, actual.size(),
        cap_match ? "True" : "False", expected_cap, actual.cap()
    );

    return memory_match && size_match && cap_match;
}

template<>
bool assert<const char *>(Array<const char *> actual, const char ** expected, size_t expected_size, size_t expected_cap) {
    bool size_match = actual.size() == expected_size;
    // make sure cap grows at the predetermined rate.
    // also make sure size doesn't exceed cap (arguably more important).
    bool cap_match = actual.cap() == expected_cap;

    bool memory_match = true;
    if (size_match) {
        for (size_t i = 0; i < expected_size; i++) {
            if (strcmp(expected[i], actual.buf()[i]) != 0) {
                memory_match = false;
            }
        }
    }
    loggerPrintf(LOGGER_TEST, "Expected:\n");
    for (size_t i = 0; i < expected_size; i++) {
        loggerPrintf(LOGGER_TEST, "%s\n", expected[i]);
    }
    loggerPrintf(LOGGER_TEST, "Actual:\n");
    for (size_t i = 0; i < actual.size(); i++) {
        loggerPrintf(LOGGER_TEST, "%s\n", actual.buf()[i]);
    }
    loggerPrintf(LOGGER_TEST, "Memory Match: %s, Size Match: %s (expected: %lu, actual: %lu), Cap Match: %s (expected: %lu, actual: %lu)\n", 
        memory_match ? "True" : "False", 
        size_match ? "True" : "False", expected_size, actual.size(),
        cap_match ? "True" : "False", expected_cap, actual.cap()
    );

    return memory_match && size_match && cap_match;
}

static void testArrayAppendCstrings(TestArg * t);
static void testArrayAppend(TestArg * t);
static void testArrayAppendRecapped(TestArg * t);
static void testArrayAppendConsecutive(TestArg * t);
static void testArrayAppendConsecutiveRecapped(TestArg * t);
static void testArrayRemoveCstrings(TestArg * t);
static void testArrayRemove(TestArg * t);
static void testArrayRemoveConsecutive(TestArg * t);
static void testArrayRemoveRecapped(TestArg * t);
static void testArrayRemoveConsecutiveRecapped(TestArg * t);

// static void testArrayInsertCstrings(TestArg * t);
// static void testArrayInsert(TestArg * t);
// static void testArrayInsertRecapped(TestArg * t);
// static void testArrayInsertConsecutive(TestArg * t);
// static void testArrayInsertConsecutiveRecapped(TestArg * t);

int main(int argc, char * argv[]) {
    Tester t;

    t.addTest(testArrayAppendCstrings);
    t.addTest(testArrayAppend);
    t.addTest(testArrayAppendRecapped);
    t.addTest(testArrayAppendConsecutive);
    t.addTest(testArrayAppendConsecutiveRecapped);

    t.addTest(testArrayRemoveCstrings);
    t.addTest(testArrayRemove);
    t.addTest(testArrayRemoveConsecutive);
    t.addTest(testArrayRemoveRecapped);
    t.addTest(testArrayRemoveConsecutiveRecapped);

    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        t.run(argv[1]);
    } else {
        t.run(nullptr);
    }

    return 0;
}

static void testArrayAppendCstrings(TestArg * t) {
    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1;
    const char * expected[ARRAY_RECOMMENDED_INITIAL_CAP] = {
        "STRING 1",
        "STRING 2",
        "STRING 3",
        "STRING 4",
        "STRING 5",
        "STRING 6",
        "STRING 7"
    };
    Array<const char *> actual(ARRAY_RECOMMENDED_INITIAL_CAP);
    actual.append(expected, expected_size);

    t->fail = !assert<const char *>(actual, expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP);
}

static void testArrayAppend(TestArg * t) {
    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1; // in this case, expected_size == expected_num_els
    uint8_t expected[expected_size];
    memset(expected, 0x07, expected_size);
    
    Array<uint8_t> actual;
    actual.append(expected, expected_size);

    t->fail = !assert<uint8_t>(actual, expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP);
}

static void testArrayAppendRecapped(TestArg * t) {
    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP + 7; // in this case, expected_size == expected_num_els - force resize
    uint8_t expected[expected_size];
    memset(expected, 0x07, expected_size);

    Array<uint8_t> actual;
    size_t initial_size = actual.size();
    actual.append(expected, expected_size);

    t->fail = !assert<uint8_t>(actual, expected, expected_size, (size_t)((expected_size + initial_size) * 1.75));
}

static void testArrayAppendConsecutive(TestArg * t) {
    Array<uint8_t> actual;

    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1; // in this case, expected_size == expected_num_els
    uint8_t expected[expected_size];

    for (int i = 0; i < expected_size; i++) {
        expected[i] = 0x07;
        actual.append(expected + i, 1);
    }

    t->fail = !assert<uint8_t>(actual, expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP);
}

static void testArrayAppendConsecutiveRecapped(TestArg * t) {
    Array<uint8_t> actual;

    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP + 7; // in this case, expected_size == expected_num_els - force resize
    uint8_t expected[expected_size];

    size_t initial_size = actual.size();

    for (int i = 0; i < expected_size; i++) {
        expected[i] = 0x07;
        actual.append(&expected[i], 1);
    }

    t->fail = !assert<uint8_t>(actual, expected, expected_size, (size_t)(ARRAY_RECOMMENDED_INITIAL_CAP + 1) * 1.75); // only one resize performed
}

static void testArrayRemoveCstrings(TestArg * t) {
    size_t expected_size = 6;
    // 
    const char * expected[expected_size] = {
        "STRING 1",
        "STRING 3",
        "STRING 4",
        "STRING 5",
        "STRING 6",
        "STRING 7"
    };
    Array<const char *> actual{
        "STRING 1",
        "STRING 2",
        "STRING 3",
        "STRING 4",
        "STRING 5",
        "STRING 6",
        "STRING 7"
    };
    size_t initial_size = actual.size();
    actual.remove(1);
    t->fail = !assert<const char *>(actual, expected, expected_size, initial_size * 1.75);
}

static void testArrayRemove(TestArg * t) {
    size_t expected_size = 6;
    uint8_t expected[expected_size] = {
        0x1,
        0x3,
        0x4,
        0x5,
        0x6,
        0x7
    };
    Array<uint8_t> actual{
        0x1,
        0x2,
        0x3,
        0x4,
        0x5,
        0x6,
        0x7
    };
    size_t initial_size = actual.size();
    actual.remove(1);
    t->fail = !assert<uint8_t>(actual, expected, expected_size, initial_size * 1.75);
}

static void testArrayRemoveConsecutive(TestArg * t) {
    size_t expected_size = 5;
    uint8_t expected[expected_size] = {
        0x1,
        0x4,
        0x5,
        0x6,
        0x7
    };
    Array<uint8_t> actual{
        0x1,
        0x2,
        0x3,
        0x4,
        0x5,
        0x6,
        0x7
    };
    size_t initial_size = actual.size();
    actual.remove(1);
    actual.remove(1);
    t->fail = !assert<uint8_t>(actual, expected, expected_size, initial_size * 1.75);
}

static void testArrayRemoveRecapped(TestArg * t) {
    size_t expected_size = 2;
    uint8_t expected[expected_size] = {
        0x1,
        0x7
    };
    Array<uint8_t> actual{
        0x1,
        0x2,
        0x3,
        0x4,
        0x5,
        0x6,
        0x7
    };
    size_t initial_size = actual.size();
    actual.remove(1, 5);
    t->fail = !assert<uint8_t>(actual, expected, expected_size, expected_size * 1.75);
}

static void testArrayRemoveConsecutiveRecapped(TestArg * t) {
    size_t expected_size = 3;
    uint8_t expected[expected_size] = {
        0x1,
        0x6,
        0x7
    };
    Array<uint8_t> actual{
        0x1,
        0x2,
        0x3,
        0x4,
        0x5,
        0x6,
        0x7
    };
    size_t initial_size = actual.size();
    actual.remove(1, 2);
    actual.remove(1, 2);
    t->fail = !assert<uint8_t>(actual, expected, expected_size, expected_size * 1.75);
}

// static void testArrayAppendCstrings(TestArg * t) {}
// static void testArrayAppend(TestArg * t) {}
// static void testArrayAppendRecapped(TestArg * t) {}
// static void testArrayAppendConsecutive(TestArg * t) {}
// static void testArrayAppendConsecutiveRecapped(TestArg * t) {}