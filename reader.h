#ifndef READER_H
#include <stdint.h>
#include <stdbool.h>

#define READ_BUFFER_SIZE 8096

typedef struct reader {
    int fd;
    // this stuff should only be modified by this module... else bad things can happen...
    uint8_t buf[READ_BUFFER_SIZE];
    uint16_t cursor; // TODO: this is probably better as uint8_t *?
    uint16_t bytes_in_buffer;
} reader;

void reader_initialize(reader * r, int fd);
int reader_peek_for_empty_line(reader * r);
uint8_t * reader_read_bytes(reader * r, uint32_t n);
char * reader_read_until(reader * r, char until);

int read_chunk_non_blocking_fd(int fd, uint8_t ** p);
#endif