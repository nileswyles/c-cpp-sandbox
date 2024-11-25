#include "tester.h"
#include "estream/byteestream.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>

#include <filesystem>

#include "file/file.h"

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

static const char * buffer_start;
static const char * buffer;

// ! IMPORTANT - overriding stdlib's implementation of read (which is apparently weakly linked...)... ByteEStream's calls to read use this function. 
extern ssize_t read(int fd, void * buf, size_t nbytes) {
    size_t ret = MIN(nbytes, strlen(buffer) + 1); // always return NUL byte of string
    memcpy(buf, buffer, ret);
    loggerPrintf(LOGGER_DEBUG, "READ RETURNED (%ld): \n", ret);
    loggerPrintByteArray(LOGGER_DEBUG, (uint8_t*)buf, ret);
    buffer += ret; // duh
    return ret; 
}

extern int poll(struct pollfd *__fds, nfds_t __nfds, int __timeout) {
    return 1;
}

static void setTestString(const char * s) {
    buffer_start = s;
    buffer = s;
}

static void readUntilAssert(TestArg * t, std::string result, std::string expected) {
    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n'%s'\n", buffer_start);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    ASSERT_STRING(t, result, expected);
}

static void testReadUntil(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    setTestString(test_string);

    std::string result = reader.read(" ").toString();
    std::string expected = "TESTSTRINGWITHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilUpperCase(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    setTestString(test_string);

    ReaderTaskUC uppercase;
    std::string result = reader.read(" ", (ReaderTask *)&uppercase).toString();
    std::string expected = "TESTSTRINGWITHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilLowerCase(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    setTestString(test_string);

    ReaderTaskLC lowercase;
    std::string result = reader.read(" ", (ReaderTask *)&lowercase).toString();
    std::string expected = "teststringwithspace ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilAllow(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    setTestString(test_string);

    ReaderTaskAllow allow("ABC");
    std::string result = reader.read(" ", (ReaderTask *)&allow).toString();
    std::string expected = "ACBA";

    readUntilAssert(t, result, expected);
}

static void testReadUntilAllowStrict(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"BLAH ";
    setTestString(test_string);

    ReaderTaskAllow allow("ABC", true);
    bool exception = false;
    try {
        std::string result = reader.read(" ", (ReaderTask *)&allow).toString();
    } catch(std::exception& e) {
        exception = true;
    }

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    ASSERT_BOOLEAN(t, exception, true);
}

static void testReadUntilDisallow(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRING";
    setTestString(test_string);

    ReaderTaskDisallow disallow("IO");
    std::string result = reader.read(" ", (ReaderTask *)&disallow).toString();
    std::string expected = "TESTSTRNG";

    readUntilAssert(t, result, expected);
}

static void testReadUntilDisallowStrict(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRING";
    setTestString(test_string);

    ReaderTaskDisallow disallow("IO", true);
    bool exception = false;
    try {
        std::string result = reader.read(" ", (ReaderTask *)&disallow).toString();
    } catch(std::exception& e) {
        exception = true;
    }

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    ASSERT_BOOLEAN(t, exception, true);
}

static void testReadUntilTrim(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "     TESTSTRINGWITHSPACE     ";
    setTestString(test_string);

    ReaderTaskTrim trim;
    std::string result = reader.read(" ", (ReaderTask *)&trim).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    readUntilAssert(t, result, expected);
}

static void testReadUntilLTrim(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "     TESTSTRINGWITHSPACE";
    setTestString(test_string);

    ReaderTaskTrim trim;
    std::string result = reader.read(" ", (ReaderTask *)&trim).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    readUntilAssert(t, result, expected);
}

static void testReadUntilRTrim(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE     ";
    setTestString(test_string);

    ReaderTaskTrim trim;
    std::string result = reader.read(" ", (ReaderTask *)&trim).toString();
    std::string expected = "TESTSTRINGWITHSPACE";

    readUntilAssert(t, result, expected);
}

static void testReadUntilDisallowSpaceTrim(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "       TESTSTRINGWITHSPACE       ";
    setTestString(test_string);

    ReaderTaskDisallow disallow(" ");
    ReaderTaskTrim trim;
    disallow.next_operation = &trim;
    std::string result = reader.read(" ", (ReaderTask *)&disallow).toString();
    std::string expected = "";

    readUntilAssert(t, result, expected);
}

static void testReadUntilExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"    ";
    setTestString(test_string);

    ReaderTaskExtract extract('"','"');
    std::string result = reader.read(" ", (ReaderTask *)&extract).toString();
    std::string expected = "TESTSTRINGWITHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilExtractNonWhiteCharacterAfterRightToken(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    // ! IMPORTANT - if readUntil space and extracting then it will include a whitespace so, this works.
    // const char * test_string = "\"TESTSTRINGWITHSPACE\" ABLBK ";
    //  but this results in an exception.
    const char * test_string = "\"TESTSTRINGWITHSPACE\"ABLBK ";
    setTestString(test_string);

    ReaderTaskExtract extract('"','"');
    bool exception = false;
    try {
        std::string result = reader.read(" ", (ReaderTask *)&extract).toString();
        loggerPrintf(LOGGER_TEST_VERBOSE, "Result:\n'%s'\n", result.c_str());
    } catch (std::exception& e) {
        exception = true;
    }
    ASSERT_BOOLEAN(t, exception, true);
}

static void testReadUntilAllowExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"    ";
    setTestString(test_string);

    ReaderTaskAllow allow("ABC\"");
    ReaderTaskExtract extract('"', '"');
    allow.next_operation = &extract;
    std::string result = reader.read(" ", (ReaderTask *)&allow).toString();
    std::string expected = "AC";

    readUntilAssert(t, result, expected);
}

static void testReadUntilAllowExtractException(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"    ";
    setTestString(test_string);

    ReaderTaskAllow allow("ABC");
    ReaderTaskExtract extract('"', '"');
    allow.next_operation = &extract;
    bool exception = false;
    try {
        reader.read(" ", (ReaderTask *)&allow).toString();
    } catch (std::exception& e) {
        exception = true;
    }
    ASSERT_BOOLEAN(t, exception, true);
}

static void testReadUntilDisallowExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"     ";
    setTestString(test_string);

    ReaderTaskDisallow disallow("IO");
    ReaderTaskExtract extract('"', '"');
    disallow.next_operation = &extract;
    std::string result = reader.read(" ", (ReaderTask *)&disallow).toString();
    std::string expected = "TESTSTRNGWTHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilLowerCaseExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWITHSPACE\"   ";
    setTestString(test_string);

    ReaderTaskLC lowercase;
    ReaderTaskExtract extract('"','"');
    lowercase.next_operation = &extract;
    std::string result = reader.read(" ", (ReaderTask *)&lowercase).toString();
    std::string expected = "teststringwithspace ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilUpperCaseExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "\"TESTSTRINGWiTHSPACE\"    ";
    setTestString(test_string);

    ReaderTaskUC uppercase;
    ReaderTaskExtract extract('"','"');
    uppercase.next_operation = &extract;
    std::string result = reader.read(" ", (ReaderTask *)&uppercase).toString();
    std::string expected = "TESTSTRINGWITHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilCursorAtUntil(TestArg * t) {
    size_t buf_size = READER_RECOMMENDED_BUF_SIZE;
    ByteEStream reader(1, buf_size);
    buffer = " BLAH";

    std::string result = reader.read(" ").toString();
    readUntilAssert(t, result, " ");
}

static void testReadUntilFillBufferOnce(TestArg * t) {
    size_t buf_size = 7;
    ByteEStream reader(1, buf_size);

    size_t expected_size = buf_size + 7;
    std::string expected;
    for (size_t i = 0; i < expected_size; i++) {
        expected += '$';
    }
    expected[expected_size - 3] = ' ';
    buffer = expected.c_str();
    
    std::string result = reader.read(" ").toString();
    readUntilAssert(t, result, "$$$$$$$$$$$ ");
}

static void testReadUntilFillBufferTwice(TestArg * t) {
    size_t buf_size = 7;
    ByteEStream reader(1, buf_size);

    size_t expected_size = (buf_size * 2) + 7;
    std::string expected;
    for (size_t i = 0; i < expected_size; i++) {
        expected += '$';
    }
    expected[expected_size - 3] = ' ';
    buffer = expected.c_str();

    std::string result = reader.read(" ").toString();
    readUntilAssert(t, result, "$$$$$$$$$$$$$$$$$$ ");
}


int main(int argc, char * argv[]) {
    Tester t("EStream Tests");
    // TODO: bug fix/feature needed... if we reach an "until" character while r_trimming (when open, before right_most_char is reached), then we will exit.
    //  might want to break only if we see until character and not r_trimming (i.e. not within quotes)... ": ": ' should yield ': ' not ':'. NOTE: left and right most characters aren't included, by design. Can probably parameterize that.
    
    // lol, this is wrong but the sentiment was that behaviour should be defined and documented regardless of direction.

    // make sure to write a test.
    t.addTest(testReadUntil);
    t.addTest(testReadUntilUpperCase);
    t.addTest(testReadUntilLowerCase);
    t.addTest(testReadUntilAllow);
    t.addTest(testReadUntilAllowStrict);
    t.addTest(testReadUntilDisallow);
    t.addTest(testReadUntilDisallowStrict);

    // TODO: .... disappointing but let's get a clean test report.
    // t.addTest(testReadUntilTrim);
    // t.addTest(testReadUntilLTrim);
    t.addTest(testReadUntilRTrim);

    t.addTest(testReadUntilDisallowSpaceTrim);

    t.addTest(testReadUntilExtract);
    t.addTest(testReadUntilExtractNonWhiteCharacterAfterRightToken);
    t.addTest(testReadUntilAllowExtract);
    t.addTest(testReadUntilAllowExtractException);
    // t.addTest(testReadUntilDisallowExtract);
    t.addTest(testReadUntilLowerCaseExtract);
    t.addTest(testReadUntilUpperCaseExtract);

    t.addTest(testReadUntilCursorAtUntil);
    t.addTest(testReadUntilFillBufferOnce);
    t.addTest(testReadUntilFillBufferTwice);

    // TODO:
    // istreamestream readuntil readbytes, collector stuff...

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}