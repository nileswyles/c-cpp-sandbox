#ifndef WYLESLIBS_READER_H
#define WYLESLIBS_READER_H

#include "array.h"

#include <string>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define READER_RECOMMENDED_BUF_SIZE 8096

namespace WylesLibs {

class ByteOperation {
    public:
        ByteOperation() {}
        void flush(Array<uint8_t> buffer, uint8_t c) {}
        virtual void perform(Array<uint8_t> buffer, uint8_t c) = 0;
};

class ByteOperationChain: public ByteOperation {
    public:
        bool ignored;
        ByteOperationChain * nextOperation;
        ByteOperationChain() {}
        ByteOperationChain(ByteOperationChain * next): nextOperation(next) {}

        void next(Array<uint8_t> buffer, uint8_t c) {
            if (this->nextOperation == nullptr) {
                if (!this->ignored) {
                    buffer.append(c);
                } else {
                    this->ignored = false;
                }
            } else {
                this->nextOperation->perform(buffer, c);
            }
        }
        virtual void perform(Array<uint8_t> buffer, uint8_t c) = 0;
};

class ByteOperationLC: public ByteOperationChain {
    public:
        void perform(Array<uint8_t> buffer, uint8_t c) {
            if (c >= 0x41 && c <= 0x5A) { // lowercase flag set and is upper case
        		c += 0x20; // lower case the char
        	}
            this->next(buffer, c);
        }
};

class ByteOperationUC: public ByteOperationChain {
    public:
        void perform(Array<uint8_t> buffer, uint8_t c) {
            if (c >= 0x61 && c <= 0x7A) {
        		c -= 0x20;
        	}
            this->next(buffer, c);
        }
};

class ByteOperationIgnore: public ByteOperationChain {
    public:
        std::string to_ignore;
        ByteOperationIgnore(std::string to_ignore): to_ignore(to_ignore) {}
        void perform(Array<uint8_t> buffer, uint8_t c) {
            if (this->to_ignore.find(c) != std::string::npos) { 
                this->ignored = true;
            }
            this->next(buffer, c);
        }
};

class ByteOperationTrim: public ByteOperation {
    public:
        Array<uint8_t> data;
        Array<uint8_t> r_trim;
        bool l_trim_ignore;

        ByteOperationTrim(): l_trim_ignore(true) {}

        void flush(Array<uint8_t> buffer, uint8_t c) {
            if (this->r_trim.getSize() > 0) {
                buffer.append(this->r_trim.buf, this->r_trim.getSize());
            }
        }
        // must be only... lol
        void perform(Array<uint8_t> buffer, uint8_t c) {
            if (!this->l_trim_ignore) {
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                    this->r_trim.append(c);
                } else if (this->r_trim.getSize() > 0) {
                    flush(buffer, c);
                } else {
                    buffer.append(c);
                }
            } else if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r')) {
                this->l_trim_ignore = false;
            }
        }
};

class Reader {
    private:
        int fd;
        uint8_t * buf;
        size_t buf_size;
        size_t cursor;
        size_t bytes_in_buffer;
        
        int fillBuffer();
        bool cursorCheck();
        // Array<uint8_t> readUntil(std::string until, ByteOperation * operation, bool is_trim);
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
        // peeking might still be useful? if we're being complete, missed a primitive? STL implements peeking functionality... 
        int peekForEmptyLine();
        Array<uint8_t> readBytes(const size_t n);
        Array<uint8_t> readUntil(const char until) {
            return readUntil(until, nullptr);
        }
        Array<uint8_t> readUntil(std::string until) {
            return readUntil(until, nullptr);
        }
        // So no overload of types within class hierarchy??
        // Array<uint8_t> readUntil(const char until, ByteOperationChain * operation) {
        //     return readUntil(std::string(&until), operation, false);
        // }
        Array<uint8_t> readUntil(const char until, ByteOperation * operation) {
            // return readUntil(std::string(&until), operation, false);
            return readUntil(std::string(&until), operation);
        }
        Array<uint8_t> readUntil(std::string until, ByteOperation * operation);
        // Array<uint8_t> readUntil(std::string until, ByteOperation * operation) {
        //     // return readUntil(until, operation, true);
        //     return readUntil(until, operation);
        // }
        // Array<uint8_t> readUntil(std::string until, ByteOperationTrim * operation) {
        //     return readUntil(until, operation, true);
        // }
        int read_chunk_non_blocking_fd(int fd, uint8_t ** p);
};
}

#endif