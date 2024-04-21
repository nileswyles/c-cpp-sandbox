#include "reader.h"
#include "logger.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifndef TEST_DEBUG
#define TEST_DEBUG 1
#endif

const char * buffer;

ssize_t read(int fd, void *buf, size_t nbytes) {
    size_t ret = MIN(nbytes, strlen(buffer) + 1); // always return NUL byte of string
    memcpy(buf, buffer, ret);
    logger_printf(LOGGER_DEBUG, "READ RETURNED (%ld): ", ret);
    logger_print_byte_array(LOGGER_DEBUG, (uint8_t*)buf, ret);
    buffer += ret; // duh
    return ret; 
}

void testReadUntil() {
    reader * reader = reader_constructor(-1, READER_RECOMMENDED_BUF_SIZE);
    printf("\nTest Func: testReadUntil\n");
    const char * test_string = "TESTSTRINGWITHSPACE BLAH";
    buffer = test_string;
    char * ret = (char *)reader_read_until(reader, ' ');
    printf("Test String:\n%s\n", test_string); // lol
    printf("Until char:\n[%x]\n", ' ');
    printf("Result:\n%s\n", ret);
    printf("Expected:\n%s\n", "TESTSTRINGWITHSPACE");
    if (strcmp(ret, "TESTSTRINGWITHSPACE") == 0) {
        printf("Test Passed!\n");
    } else {
        printf("Test Failed!\n");
    }
    reader_destructor(reader);
}

void testReadUntilCursorAtUntil() {
    reader * reader = reader_constructor(-1, READER_RECOMMENDED_BUF_SIZE);
    printf("\nTest Func: testReadUntilCursorAtUntil\n");
    buffer = " BLAH";
    char * ret = (char *)reader_read_until(reader, ' ');
    printf("Test String:\n BLAH\n"); // lol
    printf("Until char:\n[%x]\n", ' ');
    printf("Result:\n%s\n", ret);
    printf("Expected:\n%s\n", "");
    if (strcmp(ret, "") == 0) {
        printf("Test Passed!\n");
    } else {
        printf("Test Failed!\n");
    }
    reader_destructor(reader);
}

void testReadUntilFillBufferOnce() {
    reader * reader = reader_constructor(-1, 7);
    int size = reader->buf_size + 7;
    char buf[size];
    int i = 0;
    for (i = 0; i < size; i++) {
        buf[i] = '$';
    }
    buf[size - 3] = ' ';
    buf[size] = 0; // TODO: still puzzled by why this is necessary and whether or not compiler ensures it's safe.
    buffer = buf;

    char expected[size-3]; // until char should == NUL
    strncpy(expected, buf, size-3); // more lame std lib stuff? lol
    expected[size-3] = 0;

    char * ret = (char *)reader_read_until(reader, ' ');
    printf("Test String:\n%s\n", buf);
    printf("Until char:\n[%x]\n", ' ');
    printf("Result:\n%s\n", ret);
    printf("Expected:\n%s\n", expected);
    if (strcmp(ret, (char *)expected) == 0) {
        printf("Test Passed!\n");
    } else {
        printf("Expected:\n%s\n", expected);
        printf("Test Failed!\n");
    }
    reader_destructor(reader);
}

void testReadUntilFillBufferTwice() {
    reader * reader = reader_constructor(-1, 7);
    printf("\nTest Func: testReadUntilFillBufferTwice\n");
    int size = (reader->buf_size * 2) + 7;
    char buf[size];
    int i = 0;
    for (i = 0; i < size; i++) {
        buf[i] = '$';
    }
    buf[size - 3] = ' ';
    buf[size] = 0;
    buffer = buf;

    char expected[size-3]; // until char should == NUL
    strncpy(expected, buf, size-3); // more lame std lib stuff? lol
    expected[size-3] = 0;

    char * ret = (char *)reader_read_until(reader, ' ');
    printf("Test String:\n%s\n", buf);
    printf("Until char:\n[%x]\n", ' ');
    printf("Result:\n%s\n", ret);
    printf("Expected:\n%s\n", expected);
    if (strcmp(ret, expected) == 0) {
        printf("Test Passed!\n");
    } else {
        printf("Test Failed!\n");
    }
    reader_destructor(reader);
}

// void testReadUntilCursorCheck?
// void testReadUntilCursorNonZero?

// void testReadBytes?
// void testReadBytesMax?
// void testReadBytesMin?
// void testReadBytesFillBufferOnce?
// void testReadBytesFillBufferTwice?
// void testReadBytesCursorCheck?
// void testReadBytesCursorNonZero?
// void testReadBytesThenReadUntil?
// void testReadUntilThenReadBytes?
void testRunner(void func()) {
}

int main() {
    // before suite
    testRunner(testReadUntil);
    testRunner(testReadUntilCursorAtUntil);
    testRunner(testReadUntilFillBufferOnce);
    testRunner(testReadUntilFillBufferTwice);
    printf("\n#######################################\n");
    // after suite
    // testRunner(testReadUntil);
    // testReadUntil();
    // testReadUntilCursorAtUntil();
    // printf("\n#######################################\n");
    // testReadUntilFillBufferOnce();
    // printf("\n#######################################\n");
    // testReadUntilFillBufferTwice();

    return 0;
}