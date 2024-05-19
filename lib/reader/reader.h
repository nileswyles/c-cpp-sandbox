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
        // good example of "dynamic dispatch"?
        //  As I understand it, calls to ByteOperation->flush (not virtual) will call this function regardless of how it's defined in sub-classes?
        //  By contrast, calls to perform call the function defined by the class-type at creation. Regardless of any casting along the way lol.
        //  Also, the compiler throws an error if perform isn't defined in sub-classes. 
        //     *** Then there's {} vs. = 0, which is effectively the same thing? At least when return type == void? ***
        void flush(Array<uint8_t>& buffer, uint8_t c) {}
        virtual void perform(Array<uint8_t>& buffer, uint8_t c) = 0;
};

class ByteOperationChain: public ByteOperation {
    public:
        bool ignored;
        ByteOperationChain * nextOperation;
        ByteOperationChain() {}
        ByteOperationChain(ByteOperationChain * next): nextOperation(next) {}

        void next(Array<uint8_t>& buffer, uint8_t c) {
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
        virtual void perform(Array<uint8_t>& buffer, uint8_t c) = 0;
};

class ByteOperationLC: public ByteOperationChain {
    public:
        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (c >= 0x41 && c <= 0x5A) { // lowercase flag set and is upper case
        		c += 0x20; // lower case the char
        	}
            this->next(buffer, c);
        }
};

class ByteOperationUC: public ByteOperationChain {
    public:
        void perform(Array<uint8_t>& buffer, uint8_t c) {
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
        void perform(Array<uint8_t>& buffer, uint8_t c) {
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
        bool l_trimming;
        bool r_trimming;

        std::string l_trim_match;
        std::string r_trim_match;

        ByteOperationTrim(): l_trimming(true), r_trimming(false), l_trim_match("\r\n\t "), r_trim_match("\r\n\t ") {}

        // then can use this for extracting things like json keystring and valuestring...
        //      hmm... might be worth refactoring json parser to use this.  
        //      or, is reader.getBytes() then to string faster?
        //      might be worth supporting non-fd reads regardless... lol...

        // What I have works just fine for now... I guess...

        // resulting trim does not include these characters...
        ByteOperationTrim(char left_most_char, char right_most_char): 
            l_trimming(true), r_trimming(false), l_trim_match(&left_most_char), r_trim_match(&right_most_char) {}

        void rTrimFlush(Array<uint8_t>& buffer, uint8_t c) {
            if (this->r_trim.size() > 0) {
                buffer.append(this->r_trim.buf, this->r_trim.size());
            }
            r_trimming = false;
        }

        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (!this->l_trimming) {
                if (this->r_trim_match.find(c) != std::string::npos) {
                    this->r_trimming = true;
                } else if (this->r_trimming) {
                    if (this->r_trim_match.find(c) == std::string::npos) {
                        this->r_trim.append(c);
                    } else {
                        // r_trimming and see r_trim_match... flush buffer and reset trim...
                        rTrimFlush(buffer, c);
                    }
                } else {
                    buffer.append(c);
                }
            } else if (this->l_trim_match.find(c) == std::string::npos) {
                this->l_trimming = false;
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
    public:
        Reader(const int fd) : Reader(fd, READER_RECOMMENDED_BUF_SIZE) {}
        Reader(const int pFd, const size_t pBuf_size) {
            // TODO:
            // ensure buf_size > some amount and fd > 0? else return null? lol idk
            printf("LOL\n");
            buf = newCArray<uint8_t>(buf_size);
            printf("LOL\n");
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
            return readUntil(std::string(&until));
        }
        Array<uint8_t> readUntil(std::string until) {
            return readUntil(until, nullptr);
        }
        Array<uint8_t> readUntil(std::string until, bool ignore_until) {
            ByteOperationIgnore ignore(until);
            return readUntil(until, &ignore);
        }
        Array<uint8_t> readUntil(std::string until, ByteOperation * operation);
        int read_chunk_non_blocking_fd(int fd, uint8_t ** p);
};
}

#endif