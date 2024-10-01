#include "tester.h"
#include "iostream.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <filesystem>

#include "file.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifndef LOGGER_LEVEL 
#define LOGGER_LEVEL LOGGER_TEST
#endif

#ifndef LOGGER_READER_TEST
#define LOGGER_READER_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_READER_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;

static const char * buffer;

// ! IMPORTANT - overriding stdlib's implementation of read (which is apparently weakly linked...)... IOStream's calls to read use this function. 
extern ssize_t read(int fd, void *buf, size_t nbytes) {
    size_t ret = MIN(nbytes, strlen(buffer) + 1); // always return NUL byte of string
    memcpy(buf, buffer, ret);
    loggerPrintf(LOGGER_DEBUG, "READ RETURNED (%ld): \n", ret);
    loggerPrintByteArray(LOGGER_DEBUG, (uint8_t*)buf, ret);
    buffer += ret; // duh
    return ret; 
}

static void assert(TestArg * t, std::string result, std::string expected) {
    loggerPrintf(LOGGER_TEST_VERBOSE, "Result:\n%s\n", result.c_str());
    loggerPrintf(LOGGER_TEST_VERBOSE, "Expected:\n%s\n", expected.c_str());

    if (result == expected) {
        t->fail = false;
    }
}

static void testReadUntil(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    buffer = test_string;

    std::string result = reader.readUntil(' ').toString();
    std::string expected = "TESTSTRINGWITHSPACE ";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilUpperCase(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    buffer = test_string;

    ReaderTaskUC uppercase;
    std::string result = reader.readUntil(' ', (ReaderTask *)&uppercase).toString();
    std::string expected = "TESTSTRINGWITHSPACE ";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilLowerCase(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    buffer = test_string;

    ReaderTaskLC lowercase;
    std::string result = reader.readUntil(' ', (ReaderTask *)&lowercase).toString();
    std::string expected = "teststringwithspace ";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilAllow(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilDisallow(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilTrim(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilDisallowSpaceTrim(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilExtract(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilAllowExtract(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilDisallowExtract(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilLowerCaseExtract(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilUpperCaseExtract(TestArg * t) {
    IOStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    buffer = test_string;

    ReaderTaskExtract extract('"','"');
    std::string result = reader.readUntil(' ', (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(t, result, expected);
}

static void testReadUntilCursorAtUntil(TestArg * t) {
    size_t buf_size = READER_RECOMMENDED_BUF_SIZE;
    IOStream reader(1, buf_size);
    buffer = " BLAH";

    std::string result = reader.readUntil(' ').toString();
    assert(t, result, "");
}

static void testReadUntilFillBufferOnce(TestArg * t) {
    size_t buf_size = 7;
    IOStream reader(1, buf_size);

    size_t expected_size = buf_size + 7;
    std::string expected;
    for (size_t i = 0; i < expected_size; i++) {
        expected += '$';
    }
    expected[expected_size - 3] = ' ';
    buffer = expected.c_str();
    
    std::string result = reader.readUntil(' ').toString();
    assert(t, result, expected);
}

static void testReadUntilFillBufferTwice(TestArg * t) {
    size_t buf_size = 7;
    IOStream reader(1, buf_size);

    size_t expected_size = (buf_size * 2) + 7;
    std::string expected;
    for (size_t i = 0; i < expected_size; i++) {
        expected += '$';
    }
    expected[expected_size - 3] = ' ';
    buffer = expected.c_str();

    std::string result = reader.readUntil(' ').toString();
    assert(t, result, expected);
}

int main(int argc, char * argv[]) {
    Tester t("IOStream Tests");
    // TODO: bug fix/feature needed... if we reach an "until" character while r_trimming (when open, before right_most_char is reached), then we will exit.
    //  might want to break only if we see until character and not r_trimming (i.e. not within quotes)... ":": should yield :: not :. NOTE: left and right most characters aren't included, by design. Can probably parameterize that.
    // make sure to write a test.
    t.addTest(testReadUntil);
    t.addTest(testReadUntilUpperCase);
    t.addTest(testReadUntilLowerCase);
    t.addTest(testReadUntilAllow);
    t.addTest(testReadUntilDisallow);

    t.addTest(testReadUntilTrim);
    t.addTest(testReadUntilDisallowSpaceTrim);

    t.addTest(testReadUntilExtract);
    t.addTest(testReadUntilAllowExtract);
    t.addTest(testReadUntilDisallowExtract);
    t.addTest(testReadUntilLowerCaseExtract);
    t.addTest(testReadUntilUpperCaseExtract);

    t.addTest(testReadUntilCursorAtUntil);
    t.addTest(testReadUntilFillBufferOnce);
    t.addTest(testReadUntilFillBufferTwice);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}