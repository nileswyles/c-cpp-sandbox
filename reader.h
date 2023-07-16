#ifndef READER_H
#include <stdint.h>
#include <stdbool.h>

int read_bytes(int fd, uint8_t ** p, int n);
int read_chunk(int fd, uint8_t ** p);
char * read_until(int fd, char until);

#endif