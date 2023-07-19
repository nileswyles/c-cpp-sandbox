#include "reader.h"
// #include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

char * string = "TESTSTRINGWITHSPACE BLAHBLAHBLAH";

ssize_t read (int fd, void *buf, size_t nbytes) {
    ssize_t ret = MIN(nbytes, strlen(string));
    memcpy(buf, string, ret);
    printf("READ RETURNED: ");
    for (int i = 0; i < ret; i++) {
        printf("%c", ((char *)buf)[i]);
    }
    printf("\n");
    string = string + ret; // duh
    return ret; 
}

int main() {
    reader * reader = reader_constructor(-1);
    printf("%s\n", reader_read_until(reader, ' '));
    return 0;
}