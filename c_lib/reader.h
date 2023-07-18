#if defined __cplusplus
extern "C"
{
#endif

#ifndef READER_H
#define READER_H

#include <stdint.h>
#include <stdbool.h>

#define READ_BUFFER_SIZE 8096

typedef struct reader {
    int fd;
    uint8_t buf[READ_BUFFER_SIZE];
    uint16_t cursor;
    uint16_t bytes_in_buffer;
} reader;

// NOTE: ptr to reader is const... but reader isn't.
void reader_initialize(reader * const r, const int fd);
int reader_peek_for_empty_line(reader * const r);
uint8_t * reader_read_bytes(reader * const r, const uint32_t n);
char * reader_read_until(reader * const r, const char until);

int read_chunk_non_blocking_fd(int fd, uint8_t ** p);
#endif

#if defined __cplusplus
}
#endif
