#ifndef READER_H
#define READER_H

#if defined __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define READER_RECOMMENDED_BUF_SIZE 8096

typedef struct reader {
    int fd;
    uint8_t * buf;
    size_t buf_size;
    size_t cursor;
    size_t bytes_in_buffer;
} reader;

extern reader * reader_constructor(const int fd, const size_t buf_size);
extern void reader_destructor(reader * const r);

extern int reader_peek_for_empty_line(reader * const r);

// These functions return NUL-terminated byte sequences. 
//  If the caller knows the expected data does not contain a NUL byte, they can simply cast to get a c_string.
extern uint8_t * reader_read_bytes(reader * const r, const size_t n);
extern uint8_t * reader_read_until(reader * const r, const char until);

extern int read_chunk_non_blocking_fd(int fd, uint8_t ** p);

#if defined __cplusplus
}
#endif

#endif
