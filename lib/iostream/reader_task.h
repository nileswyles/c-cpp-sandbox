#ifndef WYLESLIBS_READER_TASK_H
#define WYLESLIBS_READER_TASK_H

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
class ReaderTask {
    public:
        std::string read_until;

        virtual ~ReaderTask() = default;
        // Good example of CPP OOP

        // good example of "dynamic dispatch"?
        // Virtual allows calling function defined in original type during object creation. Functions without virtual will call the function defined in the "current type".
        //
        //  Common example is to define and use a base class to cleanly select different functionality...
        //      This means that you might cast a sub-class pointer to it's base class... Virtual is required to call the functionality in the sub-class after casting to base class.
        //  Calls to ReaderTaskChain->flush (if not virtual) would call this function regardless of how it's defined in sub-classes.
        //  By contrast, calls to perform (if virtual) call the function defined by the class-type at creation. *** Regardless of any casting/type-conversion along the way ***.

        // good example of pure functions
        //     There's {} (no-op) vs. =0, 
        //      a class with a pure function (=0) is abstract and cannot be instantiated, but can be used as a pointer type.
        //  This means the compiler throws an error if it doesn't find an implementation of the pure function in sub-classes. 

        // override and final
        // These appear to be optional and more a formality thing... Restrict behaviour as much as possible if not strictly necessary...

        //  override says that function is virtual (supports dynamic dispatch) and overrides a virtual class.
        //      - enables compiler check to ensure base class function is virtual...  
        //  final says that function is virtual (supports dynamic dispatch) and cannot be overridden.
        //      - compiler error is generated if user tries to override. 

        virtual void flush(SharedArray<uint8_t>& buffer) = 0;
        virtual void perform(SharedArray<uint8_t>& buffer, uint8_t c) = 0;
};

class ReaderTaskChain: public ReaderTask {
    public:
        bool ignored;
        ReaderTask * nextOperation;

        ReaderTaskChain(): nextOperation(nullptr), ignored(false) {}
        ReaderTaskChain(ReaderTaskChain * next): nextOperation(next), ignored(false) {}
        ~ReaderTaskChain() override = default;

        void next(SharedArray<uint8_t>& buffer, uint8_t c) {
            if (!this->ignored) {
                if (this->nextOperation == nullptr) {
                    buffer.append(c);
                } else {
                    this->nextOperation->perform(buffer, c);
                }
            }
            this->ignored = false;
        }
        void flush(SharedArray<uint8_t>& buffer) override {
            this->nextOperation->flush(buffer);
        }
        void perform(SharedArray<uint8_t>& buffer, uint8_t c) override {};
};

class ReaderTaskLC: public ReaderTaskChain {
    public:
        ~ReaderTaskLC() override = default;
        void perform(SharedArray<uint8_t>& buffer, uint8_t c) final override {
            if (c >= 0x41 && c <= 0x5A) { // lowercase flag set and is upper case
        		c += 0x20; // lower case the char
        	}
            this->next(buffer, c);
        }
};

class ReaderTaskUC: public ReaderTaskChain {
    public:
        ~ReaderTaskUC() override = default;
        void perform(SharedArray<uint8_t>& buffer, uint8_t c) final override {
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
        ~ReaderTaskDisallow() override = default;

        void perform(SharedArray<uint8_t>& buffer, uint8_t c) final override {
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
        ~ReaderTaskAllow() override = default;

        void perform(SharedArray<uint8_t>& buffer, uint8_t c) final override {
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
        SharedArray<uint8_t> data;
        SharedArray<uint8_t> r_trim;
        bool l_trimming;
        bool r_trimming;

        ReaderTaskTrim(): l_trimming(true), r_trimming(false) {}
        ~ReaderTaskTrim() override = default;

        void flush(SharedArray<uint8_t>& buffer) final override {}
        void rTrimFlush(SharedArray<uint8_t>& buffer);
        void perform(SharedArray<uint8_t>& buffer, uint8_t c) final override;
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

        ReaderTaskExtract(char left_most_char, char right_most_char): 
            l_trimming(true), r_trimming(false), 
            left_most_char(left_most_char), right_most_char(right_most_char), 
            r_trim_non_whitespace(0), r_trim_read_until(0) {}
        ~ReaderTaskExtract() override = default;

        void flush(SharedArray<uint8_t>& buffer) final override;
        void rTrimFlush(SharedArray<uint8_t>& buffer);
        void perform(SharedArray<uint8_t>& buffer, uint8_t c) final override;
};
}

#endif