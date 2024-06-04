#ifndef WYLESLIBS_READER_TASK_H
#define WYLESLIBS_READER_TASK_H

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
//  and should encapsulate ReaderTask behavior for readUntil?
//  maybe extend and overwrite readline?
//
//  Also, maybe implement include filtering too...
//      can go as far as to support pseudo-regex syntax? (character class) [A-Za-z0-9\s] etc...
//      i.e. 7e[0-5] should expand to "7e012345" then just string::find(match)
//          idk, seems useful?

//      that means a parser within a parser, parsception LMAO
class ReaderTask {
    public:
        std::string read_until;
        ReaderTask() {}
        // good example of "dynamic dispatch"?
        //  As I understand it, calls to ReaderTask->flush (not virtual) will call this function regardless of how it's defined in sub-classes?
        //  By contrast, calls to perform call the function defined by the class-type at creation. Regardless of any casting along the way lol.
        //  Also, the compiler throws an error if perform isn't defined in sub-classes. 
        //     *** Then there's {} vs. = 0, which is effectively the same thing? At least when return type == void? ***
        void flush(Array<uint8_t>& buffer) {}
        virtual void perform(Array<uint8_t>& buffer, uint8_t c) = 0;
};

class ReaderTaskChain: public ReaderTask {
    public:
        bool ignored;
        ReaderTaskChain * nextOperation;
        ReaderTaskChain() {}
        ReaderTaskChain(ReaderTaskChain * next): nextOperation(next) {}

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

class ReaderTaskLC: public ReaderTaskChain {
    public:
        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (c >= 0x41 && c <= 0x5A) { // lowercase flag set and is upper case
        		c += 0x20; // lower case the char
        	}
            this->next(buffer, c);
        }
};

class ReaderTaskUC: public ReaderTaskChain {
    public:
        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (c >= 0x61 && c <= 0x7A) {
        		c -= 0x20;
        	}
            this->next(buffer, c);
        }
};

class ReaderTaskIgnore: public ReaderTaskChain {
    public:
        std::string to_ignore;
        ReaderTaskIgnore(std::string to_ignore): to_ignore(to_ignore) {}
        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (this->to_ignore.find(c) != std::string::npos) { 
                this->ignored = true;
            }
            this->next(buffer, c);
        }
};

class ReaderTaskTrim: public ReaderTask {
    public:
        Array<uint8_t> data;
        Array<uint8_t> r_trim;
        bool l_trimming;
        bool r_trimming;

        ReaderTaskTrim(): l_trimming(true), r_trimming(false) {}

        void rTrimFlush(Array<uint8_t>& buffer) {
            if (this->r_trim.size() > 0) {
                buffer.append(this->r_trim.buf, this->r_trim.size());
            }
            r_trimming = false;
        }

        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (!this->l_trimming) {
                if (STRING_UTILS_WHITESPACE.find(c) != std::string::npos) {
                    // if just trimming whitespace...
                    this->r_trimming = true;
                    this->r_trim.append(c);
                } else if (this->r_trimming) {
                    if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                        rTrimFlush(buffer);
                        buffer.append(c);
                        this->r_trimming = false;
                    } else {
                        this->r_trim.append(c);
                    }
                } else {
                    buffer.append(c);
                }
            } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                // if just trimming whitespace...
                this->l_trimming = false;
                buffer.append(c);
            }
        }
};

class ReaderTaskExtract: public ReaderTask {
    public:
        Array<uint8_t> data;
        Array<uint8_t> r_trim;
        bool l_trimming;
        bool r_trimming;
        char r_trim_non_whitespace;

        char left_most_char;
        char right_most_char;

        ReaderTaskExtract(char left_most_char, char right_most_char): 
            l_trimming(true), r_trimming(false), left_most_char(left_most_char), right_most_char(right_most_char), r_trim_non_whitespace(0) {}

        void flush(Array<uint8_t>& buffer) {
            // if extracting token and non whitespace after token throw an exception...
            if (r_trim_non_whitespace != 0) {
                std::string msg = "Found non-whitespace char right of token.";
                loggerPrintf(LOGGER_ERROR, "%s '%c'\n", msg.c_str(), r_trim_non_whitespace);
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
                if (right_most_char == c) {
                    this->r_trimming = true;
                    this->r_trim.append(c);
                } else if (this->r_trimming) {
                    if (this->right_most_char == c) {
                        // if extracting token and right_most_char found, flush and include right_most
                        // "blablbl" bblbnlbl    | == exception 
                        // "blablbl"    | == blablbl 
                        // "blablbl" " alknla| == blablbl 
                        rTrimFlush(buffer);
                        buffer.append(c);
                        this->r_trimming = false;
                        this->r_trim_non_whitespace = 0;
                    } else {
                        this->r_trim.append(c);
                        if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                            this->r_trim_non_whitespace = c;
                        }
                    }
                } else {
                    buffer.append(c);
                }
            } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                if (c == left_most_char) {
                    this->l_trimming = false;
                } else if (read_until.find(c) != std::string::npos) {
                    // include until string if that's all... because decided that why peek if can just read and return until match.
                    //  more clunky non-sense?
                    buffer.append(c);
                } else {
                    std::string msg = "Found non-whitespace char left of token.";
                    loggerPrintf(LOGGER_ERROR, "%s '%c'\n", msg.c_str(), c);
                    throw std::runtime_error(msg);
                }
            }
        }
};
}

#endif