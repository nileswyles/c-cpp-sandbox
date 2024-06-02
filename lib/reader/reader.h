#ifndef WYLESLIBS_READER_H
#define WYLESLIBS_READER_H

#include "array.h"
#include "string_utils.h"

#include <string>
#include <stdexcept>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define READER_RECOMMENDED_BUF_SIZE 8096

namespace WylesLibs {

// TODO: If I do end up using CPP's, stream stuff it will be to replace fd read calls from the c standard library... 
//  and should encapsulate ByteOperation behavior for readUntil?
//  maybe extend and overwrite readline?
//
//  Also, maybe implement include filtering too...
//      can go as far as to support pseudo-regex syntax? (character class) [A-Za-z0-9\s] etc...
//      i.e. 7e[0-5] should expand to "7e012345" then just string::find(match)
//          idk, seems useful?

//      that means a parser within a parser, parsception LMAO
class ByteOperation {
    public:
        ByteOperation() {}
        // good example of "dynamic dispatch"?
        //  As I understand it, calls to ByteOperation->flush (not virtual) will call this function regardless of how it's defined in sub-classes?
        //  By contrast, calls to perform call the function defined by the class-type at creation. Regardless of any casting along the way lol.
        //  Also, the compiler throws an error if perform isn't defined in sub-classes. 
        //     *** Then there's {} vs. = 0, which is effectively the same thing? At least when return type == void? ***
        void flush(Array<uint8_t>& buffer) {}
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
        bool r_trim_has_non_whitespace;

        char left_most_char;
        char right_most_char;

        ByteOperationTrim(): l_trimming(true), r_trimming(false), left_most_char(0), right_most_char(0), r_trim_has_non_whitespace(false) {}

        // then can use this for extracting things like json keystring and valuestring...
        //      hmm... might be worth refactoring json parser to use this.  
        //      or, is reader.getBytes() then to string faster?
        //      might be worth supporting non-fd reads regardless... lol...

        // What I have works just fine for now... I guess...

        // resulting trim does not include these characters...

        // how common is it too trim whitespace around token?, fair to implement here?
        //  fair to throw exception if non whitespace character found before token? 
        ByteOperationTrim(char left_most_char, char right_most_char): 
            l_trimming(true), r_trimming(false), left_most_char(left_most_char), right_most_char(right_most_char), r_trim_has_non_whitespace(false) {}

        void flush(Array<uint8_t>& buffer, uint8_t c) {
            // if extracting token and non whitespace after token throw an exception...
            if (right_most_char != 0 && r_trim_has_non_whitespace) {
                std::string msg = "Found non-whitespace char right of token.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
        }

        void rTrimFlush(Array<uint8_t>& buffer) {
            if (this->r_trim.size() > 0) {
                buffer.append(this->r_trim.buf, this->r_trim.size());
            }
            r_trimming = false;
        }

        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (!this->l_trimming) {
                if (this->right_most_char == 0 && STRING_UTILS_WHITESPACE.find(c) != std::string::npos) {
                    // if just trimming whitespace...
                    this->r_trimming = true;
                    this->r_trim.append(c);
                } else if (right_most_char == c) {
                    this->r_trimming = true;
                    this->r_trim.append(c);
                } else if (this->r_trimming) {
                    if (this->right_most_char == 0 && STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                        // blablbl blablabl blablbal    | == blablbl blablabl blablbal
                        // if just trimming whitespace and not whitespace char found, flush...
                        rTrimFlush(buffer);
                        buffer.append(c);
                        this->r_trimming = false;
                    } else if (this->right_most_char == c) {
                        // if extracting token and right_most_char found, flush and include right_most
                        // "blablbl" bblbnlbl    | == exception 
                        // "blablbl"    | == blablbl 
                        // "blablbl" " alknla| == blablbl 
                        rTrimFlush(buffer);
                        buffer.append(c);
                        this->r_trimming = false;
                        this->r_trim_has_non_whitespace = false;
                    } else {
                        this->r_trim.append(c);
                        if (this->right_most_char != 0 && STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                            this->r_trim_has_non_whitespace = true;
                        }
                    }
                } else {
                    buffer.append(c);
                }
            } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                if (this->left_most_char == 0) {
                    // if just trimming whitespace...
                    this->l_trimming = false;
                    buffer.append(c);
                } else if (c == left_most_char) {
                    // TODO:
                    // hmm... yeah these should be separate operations..
                    this->l_trimming = false;
                } else {
                    std::string msg = "Found non-whitespace char left of token.";
                    loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
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
        Reader(uint8_t * buf_array, const size_t pBuf_size) {
            buf = buf_array;
            buf_size = pBuf_size;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown if read past buffer. (see fillBuffer implementation)
            fd = -1;
            bytes_in_buffer = pBuf_size;
        }
        Reader(const int fd) : Reader(fd, READER_RECOMMENDED_BUF_SIZE) {}
        Reader(const int pFd, const size_t pBuf_size) {
            if (pFd < 0) {
                throw std::runtime_error("Invalid file descriptor provided.");
            }
            if (pBuf_size < 1) {
                throw std::runtime_error("Invalid buffer size provided.");
            }
            printf("Lol, oh right. buf_size requested: %ld\n", pBuf_size);
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
        uint8_t peekByte();
        // peek until doesn't make much sense with static sized buffer... so let's omit for now...
        // peek bytes cannot exceed bytes_left_in_buffer? so let's also omit...
        uint8_t readByte();
        Array<uint8_t> readBytes(const size_t n);
        Array<uint8_t> readUntil(const char until) {
            return readUntil(std::string(&until));
        }
        Array<uint8_t> readUntil(std::string until) {
            return readUntil(until, nullptr);
        }
        // TODO:
        //  hmmm.. yeah, this syntax might not be any better than calling popBack() separately... 
        //  that said, might also want to readUntil ignore but not consume until character...
        //      so maybe that's what that flag should do? Too confusing lol?
        // put back?
        Array<uint8_t> readUntil(const char until, bool ignore_until) {
            return readUntil(std::string(&until), ignore_until);
        }
        Array<uint8_t> readUntil(std::string until, bool ignore_until) {
            if (ignore_until) {
                return readUntil(until, nullptr).popBack();
            } else {
                return readUntil(until, nullptr);
            }
        }
        Array<uint8_t> readUntil(std::string until, ByteOperation * operation, bool ignore_until) {
            if (ignore_until) {
                return readUntil(until, operation).popBack();
            } else {
                return readUntil(until, operation);
            }
        }
        Array<uint8_t> readUntil(std::string until, ByteOperation * operation);
        int read_chunk_non_blocking_fd(int fd, uint8_t ** p);
};
}

#endif