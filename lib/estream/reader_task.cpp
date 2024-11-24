#include "estream/reader_task.h"

using namespace WylesLibs;

void ReaderTaskTrim::rTrimFlush() {
    if (this->r_trim.size() > 0) {
        this->collectorAccumulate(this->r_trim);
    }
    r_trimming = false;
}

void ReaderTaskTrim::perform(uint8_t& c) {
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

void ReaderTaskExtract::flush() {
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
void ReaderTaskExtract::rTrimFlush() {
    if (this->r_trim.size() > 0) {
        this->collectorAccumulate(this->r_trim);
    }
    this->r_trimming = false;
}
// TODO: bug fix/feature needed... if we reach an "until" character while r_trimming (when open, before right_most_char is reached), then we will exit.
//  might want to break only if we see until character and not r_trimming (i.e. not within quotes)... ":": should yield :: not :. NOTE: left and right most characters aren't included, by design. Can probably parameterize that.
void ReaderTaskExtract::perform(uint8_t& c) {
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
                if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos && true == this->criteriaGood(c)) {
                    // TODO: maybe make this an option...
                    this->r_trim_non_whitespace = c;
                } else if (false == this->criteriaGood(c)) {
                    this->r_trim_read_until = c;
                }
            }
        } else {
            this->collectorAccumulate(c);
        }
    } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
        if (c == left_most_char) {
            this->l_trimming = false;
        } else if (false == this->criteriaGood(c)) {
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