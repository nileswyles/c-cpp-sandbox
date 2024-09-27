#ifndef WYLESLIBS_READER_H
#define WYLESLIBS_READER_H

// TODO:
//  lol... 
//  revisit this... might be better to reader/reader_task.h and not include reader directory in build script...

#include "reader_task.h"

#include "array.h"
#include "string_utils.h"

#include <string>
#include <stdexcept>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// TODO:
// compiler flag to enable this...
#include <openssl/ssl.h>

#define READER_RECOMMENDED_BUF_SIZE 8096

namespace WylesLibs {
// TODO: leverage RAII, containerize?
class Reader {
    private:
        int r_fd;
        uint8_t * buf;
        size_t buf_size;
        size_t cursor;
        size_t bytes_in_buffer;
        SSL * ssl;
        
        void fillBuffer();
        void cursorCheck();
    public:        
        // Reader() = default;
        Reader() {}
        Reader(uint8_t * buf_array, const size_t pBuf_size) {
            buf = buf_array;
            buf_size = pBuf_size;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown if read past buffer. (see fillBuffer implementation)
            r_fd = -1;
            bytes_in_buffer = pBuf_size;
        }
        Reader(SSL * ssl) {
            buf_size = READER_RECOMMENDED_BUF_SIZE;
            cursor = 0;
            r_fd = -1;
            bytes_in_buffer = 0;
            buf = newCArray<uint8_t>(buf_size);
            ssl = ssl;
        }
        Reader(const int r_fd): Reader(r_fd, READER_RECOMMENDED_BUF_SIZE) {}
        Reader(const int pFd, const size_t pBuf_size) {
            if (pFd < 0) {
                throw std::runtime_error("Invalid file descriptor provided.");
            }
            if (pBuf_size < 1) {
                throw std::runtime_error("Invalid buffer size provided.");
            }
            buf_size = pBuf_size;
            cursor = 0;
            r_fd = pFd;
            bytes_in_buffer = 0;
            buf = newCArray<uint8_t>(buf_size);
            ssl = nullptr;
        }
        ~Reader() {
            printf("Deconstructor called...\n");
            // delete[] buf;
        }
        int fd() { return r_fd; }
        uint8_t peekByte();
        // peek until doesn't make much sense with static sized buffer... so let's omit for now...
        // peek bytes cannot exceed bytes_left_in_buffer? so let's also omit...
        uint8_t readByte();
        Array<uint8_t> readBytes(const size_t n);
        // ! IMPORTANT - inclusive means we read and consume the until character. 
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, Array provides a method to cleanly remove the until character after the fact.
        Array<uint8_t> readUntil(const char until) {
            return readUntil(std::string(&until));
        }
        Array<uint8_t> readUntil(std::string until) {
            return readUntil(until, nullptr, true);
        }
        Array<uint8_t> readUntil(const char until, bool inclusive) {
            return readUntil(std::string(&until), inclusive);
        }
        Array<uint8_t> readUntil(std::string until, bool inclusive) {
            return readUntil(until, nullptr, inclusive);
        }
        Array<uint8_t> readUntil(std::string until, ReaderTask * operation) {
            return readUntil(until, operation, true);
        }
        Array<uint8_t> readUntil(std::string until, ReaderTask * operation, bool inclusive);

        void readDecimal(double& value, size_t& digit_count);
        void readNatural(double& value, size_t& digit_count);
};
}

#endif