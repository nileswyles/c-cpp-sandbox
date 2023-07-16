#ifndef READER_H
#include <stdint.h>
#include <stdbool.h>

#define READ_BUFFER_SIZE 8096

typedef struct reader {
    int fd;
    uint8_t buf[READ_BUFFER_SIZE];
    int cursor;
    int bytes_in_buffer;
} reader;

void reader_initialize(reader * r, int fd);
uint8_t * read_bytes(reader * r, int n);
char * read_until(reader * r, char until);

int read_chunk_non_blocking_fd(int fd, uint8_t ** p);
#endif