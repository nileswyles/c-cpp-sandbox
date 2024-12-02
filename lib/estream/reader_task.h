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
//  result == tuple/array(whole_string_match, match groups...); ? i.e. entire string match is a group, group 1. 
//      then you can differentiate further through typical group matching.
//  
//  functionality to support:
//      - character classes
//      - variable wildcard matching
//      - escape sequences?
//      - groups
//      - positive/negative lookahead/behind

//  
//      - read_until shortest vs largest match vs until character vs byte count?
//      should be doable and reasonably performant with clever buffering (memoization)? I don't think you need multiple passes? 
//
//      should be fun...

typedef StreamTask<uint8_t, SharedArray<uint8_t>> ReaderTask;

template<typename RT>
class ReaderTaskChain: public StreamTask<uint8_t, RT> {
    public:
        ReaderTask * next_operation;

        ReaderTaskChain(): next_operation(nullptr) {}
        ReaderTaskChain(ReaderTaskChain * next): next_operation(next) {}
        ~ReaderTaskChain() override = default;

        void initialize() override {}

        void next(uint8_t& c) {
            if (this->next_operation == nullptr) {
                this->collectorAccumulate(c);
            } else {
                // make sure to propogate collector and criteria objects... see estream.h::streamCollect
                this->next_operation->collector = this->collector;
                this->next_operation->criteria = this->criteria;
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

template<typename RT>
class ReaderTaskLC: public ReaderTaskChain<RT> {
    public:
        ~ReaderTaskLC() override = default;
        void perform(uint8_t& c) final override {
            if (c >= 0x41 && c <= 0x5A) { // lowercase flag set and is upper case
        		c += 0x20; // lower case the char
        	}
            this->next(c);
        }
};

template<typename RT>
class ReaderTaskUC: public ReaderTaskChain<RT> {
    public:
        ~ReaderTaskUC() override = default;
        void perform(uint8_t& c) final override {
            if (c >= 0x61 && c <= 0x7A) {
        		c -= 0x20;
        	}
            this->next(c);
        }
};

template<typename RT>
class ReaderTaskDisallow: public ReaderTaskChain<RT> {
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

template<typename RT>
class ReaderTaskAllow: public ReaderTaskChain<RT> {
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

template<typename RT>
class ReaderTaskExact: public ReaderTaskChain<RT> {
    public:
        std::string match;
        size_t i;
        bool strict;
        bool is_match;

        ReaderTaskExact(): match(""), i(0) {}
        ReaderTaskExact(std::string match, bool strict = false): match(match), i(0), strict(strict) {}

        void initialize() final override {
            is_match = true;
        }
        void perform(uint8_t& c) final override {
            if (this->match.at(i++) != c) {
                is_match = false;
                if (true == strict) {
                    std::string msg("Invalid character found in exact match:");
                    loggerPrintf(LOGGER_INFO, "%s '%c', '%s'\n", msg.c_str(), c, match.c_str());
                    throw std::runtime_error(msg);
                } else {
                    this->criteriaPreemptiveBail(); // bail as soon as not match detected.
                }
            } else {
                this->next(c);
            }
        }
        void flush() override {
            i = 0;
        }
};

template<typename RT>
class ReaderTaskTrim: public StreamTask<uint8_t, RT> {
    public:
        SharedArray<uint8_t> data;
        SharedArray<uint8_t> r_trim;
        bool l_trimming;
        bool r_trimming;

        ReaderTaskTrim() = default;
        ~ReaderTaskTrim() override = default;

        void initialize() final override {
            data.remove(0, data.size());
            r_trim.remove(0, data.size());
            l_trimming = true;
            r_trimming = false;
        }
        void flush() final override {}
        void rTrimFlush();
        void perform(uint8_t& c) final override;
};

template<typename RT>
class ReaderTaskExtract: public StreamTask<uint8_t, RT> {
    public:
        SharedArray<uint8_t> data;
        SharedArray<uint8_t> r_trim;
        bool l_trimming;
        bool r_trimming;
        uint8_t r_trim_non_whitespace;
        uint8_t r_trim_read_until;

        uint8_t left_most_char;
        uint8_t right_most_char;

        ReaderTaskExtract(char left_most_char, char right_most_char): left_most_char(left_most_char), right_most_char(right_most_char) {}
        ~ReaderTaskExtract() override = default;

        void initialize() final override {
            data.remove(0, data.size());
            r_trim.remove(0, data.size());
            l_trimming = true;
            r_trimming = false;
            r_trim_non_whitespace = 0;
            r_trim_read_until = 0;
        }
        void flush() final override;
        void rTrimFlush();
        void perform(uint8_t& c) final override;
};

template<typename RT>
void ReaderTaskTrim<RT>::rTrimFlush() {
    if (this->r_trim.size() > 0) {
        this->collectorAccumulate(this->r_trim);
    }
    r_trimming = false;
}

template<typename RT>
void ReaderTaskTrim<RT>::perform(uint8_t& c) {
    if (!this->l_trimming) {
        if (STRING_UTILS_WHITESPACE.find(c) != std::string::npos) {
            // if just trimming whitespace...
            this->r_trimming = true;
            this->r_trim.append(c);
        } else if (this->r_trimming) {
            if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                rTrimFlush();
                this->r_trimming = false;
                this->collectorAccumulate(c);
            } else {
                // store whitespaces in r_trim buffer for flushing if we see non-whitespace.
                this->collectorAccumulate(c);
            }
        } else {
            this->collectorAccumulate(c);
        }
    } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
        // if just trimming whitespace...
        this->l_trimming = false;
        this->collectorAccumulate(c);
    }
}

template<typename RT>
void ReaderTaskExtract<RT>::flush() {
    // if extracting token and non whitespace after token throw an exception...
    if (r_trim_non_whitespace != 0) {
        // TODO: maybe make this an option...
        std::string msg = "Found non-whitespace char right of token.";
        loggerPrintf(LOGGER_INFO, "%s '%c'\n", msg.c_str(), r_trim_non_whitespace);
        throw std::runtime_error(msg);
    } else if (!this->l_trimming && !this->r_trimming) {
        std::string msg = "Found open ended token.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    } else if (this->r_trim_read_until != 0) {
        this->collectorAccumulate(this->r_trim_read_until);
    }
    this->initialize();
}
template<typename RT>
void ReaderTaskExtract<RT>::rTrimFlush() {
    if (this->r_trim.size() > 0) {
        this->collectorAccumulate(this->r_trim);
    }
    this->r_trimming = false;
}
// TODO: bug fix/feature needed... if we reach an "until" character while r_trimming (when open, before right_most_char is reached), then we will exit.
//  might want to break only if we see until character and not r_trimming (i.e. not within quotes)... ":": should yield :: not :. NOTE: left and right most characters aren't included, by design. Can probably parameterize that.
template<typename RT>
void ReaderTaskExtract<RT>::perform(uint8_t& c) {
    if (!this->l_trimming) {
        if (this->right_most_char == c) {
            this->r_trimming = true;
            this->r_trim.append(c);
        } else if (this->r_trimming) {
            if (this->right_most_char == c) {
                // if extracting token and right_most_char found, flush and include right_most
                // TODO: maybe make this an option...
                // "blablbl" bblbnlbl    | == exception 
                // "blablbl"    | == blablbl 
                // "blablbl" " alknla| == blablbl - SEE TODO above... this might change.
                rTrimFlush();
                this->r_trim.append(c);
                // this->collectorAccumulate(c);
                // this->r_trimming = false;
                this->r_trim_non_whitespace = 0;
            } else {
                this->r_trim.append(c);

                if (this->criteriaState() & LOOP_CRITERIA_STATE_AT_LAST) {
                    this->r_trim_read_until = c;
                } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
                    // if not last character in stream and not whitespace

                    // TODO: maybe make this an option...
                    this->r_trim_non_whitespace = c;
                }
            }
        } else {
            this->collectorAccumulate(c);
        }
    } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
        if (c == left_most_char) {
            this->l_trimming = false;
        } else if (this->criteriaState() & LOOP_CRITERIA_STATE_AT_LAST) {
            // include until string if that's all... because decided that why peek if can just read and return until match.
            //  more clunky non-sense?
            this->collectorAccumulate(c);
        } else {
            std::string msg = "Found non-whitespace char left of token.";
            loggerPrintf(LOGGER_INFO, "%s '%c'\n", msg.c_str(), c);
            throw std::runtime_error(msg);
        }
    }
}
}

#endif