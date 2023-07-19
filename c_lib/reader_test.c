#include "reader.h"
// #include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifndef TEST_DEBUG
#define TEST_DEBUG 1
#endif

char * buffer;

ssize_t read(int fd, void *buf, size_t nbytes) {
    ssize_t ret = MIN(nbytes, strlen(buffer));
    memcpy(buf, buffer, ret);
    if (TEST_DEBUG) {
        printf("READ RETURNED (%ld): ", ret);
        for (int i = 0; i < ret; i++) {
            printf("%c", ((char *)buf)[i]);
        }
        printf("\n");
    }
    buffer = buffer + ret; // duh
    return ret; 
}

void testReadUntil() {
    reader * reader = reader_constructor(-1);
    printf("\nTest Func: testReadUntil\n");
    buffer = "TESTSTRINGWITHSPACE BLAHBLAHBLAH";
    char * ret = (char *)reader_read_until(reader, ' ');
    if (TEST_DEBUG) {
        printf("Test String:\n%s\n", buffer); // lol
        printf("Result:\n%s = readUntil(' ')\n", ret);
    }
    if (strcmp(ret, "TESTSTRINGWITHSPACE") == 0) {
        printf("Test Passed!\n");
    } else {
        printf("Test Failed!\n");
    }
}

void testReadUntilFillBufferOnce() {
    reader * reader = reader_constructor(-1);
    printf("\nTest Func: testReadUntilFillBufferOnce\n");
    // 8103 = READ_BUFFER_SIZE + 7 because no malloc.
    char buf[8103];
    int i = 0;
    for (i = 0; i < 8103; i++) {
        buf[i] = '$';
    }
    buf[8099] = ' ';
    buf[8102] = 0;
    buffer = buf;

    char expected[8100];
    strncpy(expected, buf, 8100); // more lame std lib stuff? lol
    expected[8099] = 0;

    char * ret = (char *)reader_read_until(reader, ' ');
    if (TEST_DEBUG) {
        printf("Test String:\n%s\n", buffer);
        printf("Result:\n%s = readUntil(' ')\n", ret);
    }
    if (strcmp(ret, expected) == 0) {
        printf("Test Passed!\n");
    } else {
        printf("Test Failed!\n");
    }
}

int main() {
    testReadUntil();
    testReadUntilFillBufferOnce();

    return 0;
}