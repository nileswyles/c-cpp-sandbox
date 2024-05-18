#pragma once // instead of ifndef guard?

#include "array.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define READER_RECOMMENDED_BUF_SIZE 8096

namespace WylesLibs::IO {

typedef void(ByteOperation)(uint8_t&);

void byteOperationToLowerCase(uint8_t& c) {
    if (c >= 0x41 && c <= 0x5A) { // lowercase flag set and is upper case
		c += 0x20; // lower case the char
	}
}

void byteOperationIgnoreWhiteSpace(uint8_t& c) {
    if (c == '\t' || c == ' ') { 
        c = '';
    }
}

void byteOperationToLowerCaseAndIgnoreWhiteSpace(uint8_t& c) {
    byteOperationToLowerCase(c);
    byteOperationIgnoreWhiteSpace(c);
}

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
        Array<uint8_t> * readUntil(const char until) {
            return readUntil(until, nullptr);
        }
        Array<uint8_t> * readUntil(std::string until) {
            return readUntil(until, nullptr);
        }
        Array<uint8_t> * readUntil(const char until, ByteOperation * operation) {
            return readUntil(std::string(until), operation)
        }
        Array<uint8_t> * readUntil(std::string until, ByteOperation * operation);
        int read_chunk_non_blocking_fd(int fd, uint8_t ** p);
};
}