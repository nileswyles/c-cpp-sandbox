#include "estream/reader_task.h"

using namespace WylesLibs;

void ReaderTaskTrim::rTrimFlush(SharedArray<uint8_t>& buffer) {
    if (this->r_trim.size() > 0) {
        buffer.append(this->r_trim);
    }
    r_trimming = false;
}

void ReaderTaskTrim::perform(SharedArray<uint8_t>& buffer, uint8_t c) {
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
                // store whitespaces in r_trim buffer for flushing if we see non-whitespace.
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

void ReaderTaskExtract::flush(SharedArray<uint8_t>& buffer) {
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
        buffer.append(this->r_trim_read_until);
    }
}
void ReaderTaskExtract::rTrimFlush(SharedArray<uint8_t>& buffer) {
    if (this->r_trim.size() > 0) {
        buffer.append(this->r_trim);
    }
    this->r_trimming = false;
}
// TODO: bug fix/feature needed... if we reach an "until" character while r_trimming (when open, before right_most_char is reached), then we will exit.
//  might want to break only if we see until character and not r_trimming (i.e. not within quotes)... ":": should yield :: not :. NOTE: left and right most characters aren't included, by design. Can probably parameterize that.
void ReaderTaskExtract::perform(SharedArray<uint8_t>& buffer, uint8_t c) {
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
                rTrimFlush(buffer);
                this->r_trim.append(c);
                // buffer.append(c);
                // this->r_trimming = false;
                this->r_trim_non_whitespace = 0;
            } else {
                this->r_trim.append(c);
                if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos && read_until.find(c) == std::string::npos) {
                    // TODO: maybe make this an option...
                    this->r_trim_non_whitespace = c;
                } else if (read_until.find(c) != std::string::npos) {
                    this->r_trim_read_until = c;
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
            loggerPrintf(LOGGER_INFO, "%s '%c'\n", msg.c_str(), c);
            throw std::runtime_error(msg);
        }
    }
}