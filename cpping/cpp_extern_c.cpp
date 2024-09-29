#include <unistd.h>
#include <cstdio>
#include "iostream.h"

int main() {
    int * blah[10];
    ssize_t ret = read(-1, blah, 1);
    printf("%ld\n", ret);
    reader reader = {0}; // explicitly initialize to zero, I think {0} == {};
    reader_initialize(&reader, -1);
    reader.cursor = -7;
    printf("%d, %d\n", reader.fd, reader.cursor);
    return 0;
}