#ifndef WYLESLIBS_READER_TASK_H
#define WYLESLIBS_READER_TASK_H

#include "array.h"
#include "string_utils.h"

#include <string>
#include <stdexcept>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_READER_TASK
#define LOGGER_LEVEL_READER_TASK GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_READER_TASK
#define LOGGER_READER_TASK 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_READER_TASK

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_READER_TASK
#include "logger.h"

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
        //  As I understand it, calls to ReaderTaskChain->flush (not virtual) will call this function regardless of how it's defined in sub-classes?
        //  By contrast, calls to perform call the function defined by the class-type at creation. Regardless of any casting along the way lol.
        //  Also, the compiler throws an error if perform isn't defined in sub-classes. 
        //     *** Then there's {} vs. = 0, which is effectively the same thing? At least when return type == void? ***
        virtual void flush(Array<uint8_t>& buffer) = 0;
        virtual void perform(Array<uint8_t>& buffer, uint8_t c) = 0;
};

class ReaderTaskChain: public ReaderTask {
    public:
        bool ignored;
        ReaderTask * nextOperation;

        ReaderTaskChain(): nextOperation(nullptr), ignored(false) {}
        ReaderTaskChain(ReaderTaskChain * next): nextOperation(next), ignored(false) {}

        void next(Array<uint8_t>& buffer, uint8_t c) {
            if (!this->ignored) {
                if (this->nextOperation == nullptr) {
                    buffer.append(c);
                } else {
                    this->nextOperation->perform(buffer, c);
                }
            }
            this->ignored = false;
        }
        void flush(Array<uint8_t>& buffer) {
            this->nextOperation->flush(buffer);
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

class ReaderTaskDisallow: public ReaderTaskChain {
    public:
        std::string to_disallow;
        bool strict;

        ReaderTaskDisallow(std::string to_disallow): to_disallow(to_disallow), strict(false) {}
        ReaderTaskDisallow(std::string to_disallow, bool strict): to_disallow(to_disallow), strict(strict) {}

        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (this->to_disallow.find(c) != std::string::npos) { 
                if (strict) {
                    std::string msg = "Banned character found:";
                    loggerPrintf(LOGGER_ERROR, "%s '%c'\n", msg.c_str(), c);
                    throw std::runtime_error(msg);
                } else {
                    this->ignored = true;
                }
            }
            this->next(buffer, c);
        }
};

class ReaderTaskAllow: public ReaderTaskChain {
    public:
        std::string to_allow;
        bool strict;

        ReaderTaskAllow(std::string to_allow): to_allow(to_allow), strict(false) {}
        ReaderTaskAllow(std::string to_allow, bool strict): to_allow(to_allow), strict(strict) {}

        void perform(Array<uint8_t>& buffer, uint8_t c) {
            if (this->to_allow.find(c) == std::string::npos) { 
                if (strict) {
                    std::string msg = "Banned character found:";
                    loggerPrintf(LOGGER_ERROR, "%s '%c'\n", msg.c_str(), c);
                    throw std::runtime_error(msg);
                } else {
                    this->ignored = true;
                }
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

        void rTrimFlush(Array<uint8_t>& buffer);
        void perform(Array<uint8_t>& buffer, uint8_t c);
};

class ReaderTaskExtract: public ReaderTask {
    public:
        Array<uint8_t> data;
        Array<uint8_t> r_trim;
        bool l_trimming;
        bool r_trimming;
        uint8_t r_trim_non_whitespace;
        uint8_t r_trim_read_until;

        uint8_t left_most_char;
        uint8_t right_most_char;

        ReaderTaskExtract(char left_most_char, char right_most_char): 
            l_trimming(true), r_trimming(false), 
            left_most_char(left_most_char), right_most_char(right_most_char), 
            r_trim_non_whitespace(0), r_trim_read_until(0) {}

        void flush(Array<uint8_t>& buffer);
        void rTrimFlush(Array<uint8_t>& buffer);
        void perform(Array<uint8_t>& buffer, uint8_t c);
};
}

#endif