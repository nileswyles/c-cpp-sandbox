#include "reader.h"
// #include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifndef TEST_DEBUG
#define TEST_DEBUG 1
#endif

char * buffer = "";

ssize_t read(int fd, void *buf, size_t nbytes) {
    ssize_t ret = MIN(nbytes, strlen(buffer));
    memcpy(buf, buffer, ret);
    if (TEST_DEBUG) {
        printf("READ RETURNED: ");
        for (int i = 0; i < ret; i++) {
            printf("%c", ((char *)buf)[i]);
        }
        printf("\n");
    }
    buffer = buffer + ret; // duh
    return ret; 
}

void testReadUntil(reader * reader) {
    printf("Test Func: testReadUntil\n");
    buffer = "TESTSTRINGWITHSPACE BLAHBLAHBLAH";
    char * ret = (char *)reader_read_until(reader, ' ');
    if (TEST_DEBUG) {
        printf("Test String: %s\n", buffer);
        printf("%s = readUntil(' ')\n", ret);
    }
    if (strcmp(ret, "TESTSTRINGWITHSPACE") == 0) {
        printf("Test Passed!\n");
    } else {
        printf("Test Failed!\n");
    }
}

int main() {
    reader * reader = reader_constructor(-1);
    testReadUntil(reader);
    return 0;
}