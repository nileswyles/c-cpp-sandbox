#include "tester.h"
#include "reader.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

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

ssize_t read(int fd, void *buf, size_t nbytes) {
    size_t ret = MIN(nbytes, strlen(buffer) + 1); // always return NUL byte of string
    memcpy(buf, buffer, ret);
    loggerPrintf(LOGGER_DEBUG, "READ RETURNED (%ld): \n", ret);
    loggerPrintByteArray(LOGGER_DEBUG, (uint8_t*)buf, ret);
    buffer += ret; // duh
    return ret; 
}

void assert(std::string result, std::string expected) {
    loggerPrintf(LOGGER_TEST_VERBOSE, "Result:\n%s\n", result.c_str());
    loggerPrintf(LOGGER_TEST_VERBOSE, "Expected:\n%s\n", expected.c_str());

    if (result == expected) {
        // t->fail = false;
    }
}

// void testReadUntil(TestArg * t) {
void testReadUntil() {
    Reader reader(-1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    buffer = test_string;

    std::string ret = reader.readUntil(' ').toString();
    std::string expected = "TESTINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(ret, expected);
}

void testReadUntilUpperCase() {
    Reader reader(-1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    buffer = test_string;

    ByteOperationUC uppercase;
    std::string ret = reader.readUntil(' ', (ByteOperation *)&uppercase).toString();
    std::string expected = "TESTINGWITHSPACE";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(ret, expected);
}

void testReadUntilLowerCase() {
    Reader reader(-1, READER_RECOMMENDED_BUF_SIZE);

    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    buffer = test_string;

    ByteOperationLC lowercase;
    std::string ret = reader.readUntil(' ', (ByteOperation *)&lowercase).toString();
    std::string expected = "testingwithspace";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", test_string);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    assert(ret, expected);
}

// void testReadUntilCursorAtUntil() {
//     reader * reader = reader_constructor(-1, READER_RECOMMENDED_BUF_SIZE);
//     printf("\nTest Func: testReadUntilCursorAtUntil\n");
//     buffer = " BLAH";
//     char * ret = (char *)reader_read_until(reader, ' ');
//     printf("Test String:\n BLAH\n"); // lol
//     printf("Until char:\n[%x]\n", ' ');
//     printf("Result:\n%s\n", ret);
//     printf("Expected:\n%s\n", "");
//     if (strcmp(ret, "") == 0) {
//         printf("Test Passed!\n");
//     } else {
//         printf("Test Failed!\n");
//     }
//     reader_destructor(reader);
// }

// void testReadUntilFillBufferOnce() {
//     reader * reader = reader_constructor(-1, 7);
//     int size = reader->buf_size + 7;
//     char buf[size];
//     int i = 0;
//     for (i = 0; i < size; i++) {
//         buf[i] = '$';
//     }
//     buf[size - 3] = ' ';
//     buf[size] = 0; // TODO: still puzzled by why this is necessary and whether or not compiler ensures it's safe.
//     buffer = buf;

//     char expected[size-3]; // until char should == NUL
//     strncpy(expected, buf, size-3); // more lame std lib stuff? lol
//     expected[size-3] = 0;

//     char * ret = (char *)reader_read_until(reader, ' ');
//     printf("Test String:\n%s\n", buf);
//     printf("Until char:\n[%x]\n", ' ');
//     printf("Result:\n%s\n", ret);
//     printf("Expected:\n%s\n", expected);
//     if (strcmp(ret, (char *)expected) == 0) {
//         printf("Test Passed!\n");
//     } else {
//         printf("Expected:\n%s\n", expected);
//         printf("Test Failed!\n");
//     }
//     reader_destructor(reader);
// }

// void testReadUntilFillBufferTwice() {
//     reader * reader = reader_constructor(-1, 7);
//     printf("\nTest Func: testReadUntilFillBufferTwice\n");
//     int size = (reader->buf_size * 2) + 7;
//     char buf[size];
//     int i = 0;
//     for (i = 0; i < size; i++) {
//         buf[i] = '$';
//     }
//     buf[size - 3] = ' ';
//     buf[size] = 0;
//     buffer = buf;

//     char expected[size-3]; // until char should == NUL
//     strncpy(expected, buf, size-3); // more lame std lib stuff? lol
//     expected[size-3] = 0;

//     char * ret = (char *)reader_read_until(reader, ' ');
//     printf("Test String:\n%s\n", buf);
//     printf("Until char:\n[%x]\n", ' ');
//     printf("Result:\n%s\n", ret);
//     printf("Expected:\n%s\n", expected);
//     if (strcmp(ret, expected) == 0) {
//         printf("Test Passed!\n");
//     } else {
//         printf("Test Failed!\n");
//     }
//     reader_destructor(reader);
// }

int main(int argc, char * argv[]) {
    // Tester t;

    // testReadUntil();
    // testReadUntilUpperCase();
    testReadUntilLowerCase();
    // t.addTest(testReadUntil);
    // addTest(testReadUntilCursorAtUntil);
    // addTest(testReadUntilFillBufferOnce);
    // addTest(testReadUntilFillBufferTwice);

    // if (argc > 1) {
    //     loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
    //     t.run(argv[1]);
    // } else {
    //     t.run(nullptr);
    // }

    return 0;
}