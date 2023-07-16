#ifndef READER_H
#include <stdint.h>
#include <stdbool.h>

#define READ_BUFFER_SIZE 8096

typedef struct reader {
    int fd;
    uint8_t buf[READ_BUFFER_SIZE];
    int cursor;
} reader;

void reader_initialize(reader * r, int fd);
int read_bytes(reader * r, uint8_t ** p, int n);
int read_chunk(reader * r, uint8_t ** p);
char * read_until(reader * r, char until);

#endif