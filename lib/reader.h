#pragma once // instead of ifndef guard?

#include "array.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define READER_RECOMMENDED_BUF_SIZE 8096

namespace WylesLibs {
class Reader {
    private:
        int fd;
        uint8_t * buf;
        size_t buf_size;
        size_t cursor;
        size_t bytes_in_buffer;
        
        int fillBuffer();
        bool cursorCheck();

    public:
        Reader(const int fd) : Reader(fd, READER_RECOMMENDED_BUF_SIZE) {}
        Reader(const int pFd, const size_t pBuf_size) {
            // TODO:
            // ensure buf_size > some amount and fd > 0? else return null? lol idk
            buf = newCArray<uint8_t>(buf_size);
            buf_size = pBuf_size;
            cursor = 0;
            fd = pFd;
            bytes_in_buffer = 0;
        }
        ~Reader() {
            delete buf;
        }
        int peekForEmptyLine();
        Array<uint8_t> * readBytes(const size_t n);
        Array<uint8_t> * readUntil(const char until);
        int read_chunk_non_blocking_fd(int fd, uint8_t ** p);
};
}