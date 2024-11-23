#ifndef WYLESLIBS_READER_TASK_H
#define WYLESLIBS_READER_TASK_H

#include "estream/estream_types.h"
#include "datastructures/array.h"
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

//  TODO:
//  Maybe implement include filtering...
//      can go as far as to support pseudo-regex syntax? (character class) [A-Za-z0-9\s] etc...
//      i.e. 7e[0-5] should expand to "7e012345" then just string::find(match)
//          idk, seems useful?

//      that means a parser within a parser, parsception LMAO

typedef StreamTask<uint8_t, SharedArray<uint8_t>> ReaderTask;

class ReaderTaskChain: public ReaderTask {
    public:
        StreamTask<uint8_t, SharedArray<uint8_t>> * next_operation;

        ReaderTaskChain(): next_operation(nullptr) {}
        ReaderTaskChain(ReaderTaskChain * next): next_operation(next) {}
        ~ReaderTaskChain() override = default;

        void next(uint8_t& c) {
            if (this->next_operation == nullptr) {
                this->collectorAccumulate(c);
            } else {
                return this->next_operation->perform(c);
            }
        }
        void flush() override {
            if (this->next_operation != nullptr) {
                this->next_operation->flush();
            }
        }
        virtual void perform(uint8_t& c) = 0;
};

class ReaderTaskLC: public ReaderTaskChain {
    public:
        ~ReaderTaskLC() override = default;
        void perform(uint8_t& c) final override {
            if (c >= 0x41 && c <= 0x5A) { // lowercase flag set and is upper case
        		c += 0x20; // lower case the char
        	}
            this->next(c);
        }
};

class ReaderTaskUC: public ReaderTaskChain {
    public:
        ~ReaderTaskUC() override = default;
        void perform(uint8_t& c) final override {
            if (c >= 0x61 && c <= 0x7A) {
        		c -= 0x20;
        	}
            this->next(c);
        }
};

class ReaderTaskDisallow: public ReaderTaskChain {
    public:
        std::string to_disallow;
        bool strict;

        ReaderTaskDisallow(): to_disallow(" \t"), strict(false) {}
        ReaderTaskDisallow(std::string to_disallow): to_disallow(to_disallow), strict(false) {}
        ReaderTaskDisallow(std::string to_disallow, bool strict): to_disallow(to_disallow), strict(strict) {}
        ~ReaderTaskDisallow() override = default;

        void perform(uint8_t& c) final override {
            bool ignored = false;
            if (this->to_disallow.find(c) != std::string::npos) { 
                if (strict) {
                    std::string msg = "Banned character found:";
                    loggerPrintf(LOGGER_INFO, "%s '%c'\n", msg.c_str(), c);
                    throw std::runtime_error(msg);
                } else {
                    ignored = true;
                }
            } 
            if (false == ignored) {
                this->next(c);
            }
        }
};

class ReaderTaskAllow: public ReaderTaskChain {
    public:
        std::string to_allow;
        bool strict;

        // allow alphanumeric by default because why not? 
        ReaderTaskAllow(): to_allow("abcdefghijklmnopqrstuvwxyz012345679ABCDEFGHIJKLMNOPQRSTUVWXYZ"), strict(false) {}
        ReaderTaskAllow(std::string to_allow): to_allow(to_allow), strict(false) {}
        ReaderTaskAllow(std::string to_allow, bool strict): to_allow(to_allow), strict(strict) {}
        ~ReaderTaskAllow() override = default;

        void perform(uint8_t& c) final override {
            if (this->to_allow.find(c) != std::string::npos) { 
                this->next(c);
            } else {
                if (strict) {
                    std::string msg = "Banned character found:";
                    loggerPrintf(LOGGER_INFO, "%s '%c'\n", msg.c_str(), c);
                    throw std::runtime_error(msg);
                }
            }
        }
};

class ReaderTaskTrim: public ReaderTask {
    public:
        SharedArray<uint8_t> data;
        SharedArray<uint8_t> r_trim;
        bool l_trimming;
        bool r_trimming;

        ReaderTaskTrim(): l_trimming(true), r_trimming(false) {}
        ~ReaderTaskTrim() override = default;

        void flush() final override {}
        void rTrimFlush();
        void perform(uint8_t& c) final override;
};

class ReaderTaskExtract: public ReaderTask {
    public:
        SharedArray<uint8_t> data;
        SharedArray<uint8_t> r_trim;
        bool l_trimming;
        bool r_trimming;
        uint8_t r_trim_non_whitespace;
        uint8_t r_trim_read_until;

        uint8_t left_most_char;
        uint8_t right_most_char;

        // TODO: might be good to initialize these (just read_until?) to something other than NUL in case that's the character... maybe some >128 but not 255...
        ReaderTaskExtract(char left_most_char, char right_most_char): 
            l_trimming(true), r_trimming(false), 
                                                                      left_most_char(left_most_char), right_most_char(right_most_char), 
                                                                      r_trim_non_whitespace(0), r_trim_read_until(0) {}
        ~ReaderTaskExtract() override = default;

        void flush() final override;
        void rTrimFlush();
        void perform(uint8_t& c) final override;
};
}

#endif