#ifndef WYLESLIBS_STRING_UTILS_H
#define WYLESLIBS_STRING_UTILS_H

#include <math.h>
#include <string>
#include <stdbool.h>
#include <stdexcept>
#include <stdint.h>

namespace WylesLibs {
    static std::string STRING_UTILS_WHITESPACE = "\r\n\t ";
    
    static bool isAlpha(char c) {
        if ((c >= 0x41 && c <= 0x5A) || (c >=0x61 && c <= 0x7A)) {
            return true;
        }
        return false;
    }
    
    static bool isDigit(char c) {
        if (c >= 0x30 && c <= 0x39) {
            return true;
        }
        return false;
    }
    
    static bool isLowerHex(char c) {
        if (c >= 0x61 && c <= 0x66) {
            return true;
        }
        return false;
    }
    
    static bool isUpperHex(char c) {
        if (c >= 0x41 && c <= 0x46) {
            return true;
        }
        return false;
    }
    
    static bool isHexDigit(char c) {
        return isDigit(c) || isLowerHex(c) || isUpperHex(c);
    }
    
    static char hexToChar(std::string buf) {
        char ret = 0x00;
        for (size_t i = 0; i < 2; i++) {
            ret = ret << 4 | buf.at(i);
        }
        return ret;
    }

    class StringFormatOpts {
        public:
            static const uint8_t NO_SIGN = 0x0;
            static const uint8_t SIGN_FOR_NEGATIVES = 0x1;
            static const uint8_t SIGN_FOR_POSITIVES = 0x2;

            uint8_t base;
            uint8_t width;
            uint8_t precision;
            int8_t exponential;
            bool upper;
            uint8_t sign_mask;
            bool is_negative;

            StringFormatOpts(): base(10), width(UINT8_MAX), precision(6), exponential(0), upper(true), sign_mask(SIGN_FOR_NEGATIVES), is_negative(false) {}
    };

    static std::string numToString(uint64_t num, StringFormatOpts opts) {
        std::string s;
        size_t divisor = 1;
        char digit;
        size_t i = 0;

        if (opts.base != 10 && opts.base != 16) {
            throw std::runtime_error("Base not supported.");
        }
        if (true == opts.is_negative) {
            if ((opts.sign_mask & StringFormatOpts::SIGN_FOR_NEGATIVES) > 0) {
                s += '-';
            }
        } else {
            if ((opts.sign_mask & StringFormatOpts::SIGN_FOR_POSITIVES) > 0) {
                s += '+';
            }
        }
        if (opts.base == 16) {
            s += "0x";
        }
        size_t digit_count = 0;
        while (num / divisor > opts.base) {
            divisor *= opts.base;
            digit_count++;
        }
        size_t start_index = 0;
        if (opts.width != UINT8_MAX && digit_count < opts.width) {
            // pad to width
            opts.width -= digit_count;
            while (opts.width > 0) {
                s += '0';
            }
        } else if (digit_count > opts.width) {
            // truncate
            start_index = digit_count - opts.width;
        }
        while (divisor > 0) {
            digit = (char)(num / divisor);
            if (i >= start_index) { // truncate
                if (digit <= 9) {
                    s += digit + '0';
                } else if (opts.base == 16 && digit <= 0xF) {
                    if (true == opts.upper) {
                        s += digit - 0xA + 'A';
                    } else {
                        s += digit - 0xA + 'a';
                    }
                } else {
                    throw std::runtime_error("Invalid digit character detected.");
                }
            }
            num -= (digit * divisor);
            divisor /= opts.base;
            i++;
        }
        return s;
    }

    // TODO: octal
    static std::string numToStringSigned(int64_t num, StringFormatOpts opts) {
        if (num < 0) {
            num *= -1;
            opts.is_negative = true;
        }
        return numToString(static_cast<uint64_t>(num), opts);
    }

    static std::string floatToString(double num, StringFormatOpts opts) {
        std::string s;
        if (num < 0) {
            if ((opts.sign_mask & StringFormatOpts::SIGN_FOR_NEGATIVES) > 0) {
                s += '-';
            }
        } else {
            if ((opts.sign_mask & StringFormatOpts::SIGN_FOR_POSITIVES) > 0) {
                s += '+';
            }
        }
        int16_t precision_count = -1;
        size_t divisor = 1;
        size_t digit_count = 0;
        size_t start_index = 0;
        size_t decimal_idx;
        char digit;
        size_t i = 0;

        if (opts.precision == 0) {
            opts.precision = 1;
        }
        // process exponential
        if (opts.exponential == 0) {
            decimal_idx = pow(10, opts.precision);
            num *= decimal_idx;
        } else if (opts.exponential > 0) {
            size_t width_divisor = 1;
            size_t detected_width = 1;
            while (num / width_divisor > 10) {
                width_divisor *= 10;
                detected_width++;
            }
            if (static_cast<int64_t>(detected_width) < opts.exponential) {
                s += "0.";
                size_t pad_count = opts.exponential - detected_width;
                bool zeroed_result = false;
                if (opts.precision > pad_count) {
                    precision_count = pad_count + 1;
                } else {
                    pad_count = opts.precision;
                    zeroed_result = true;
                }
                while (pad_count > 0) {
                    s += "0";
                    pad_count--;
                }
                if (true == zeroed_result) {
                    return s;
                }
            } else {
                decimal_idx = pow(10, opts.precision + opts.exponential);
            }
            num *= pow(10, opts.precision);
        } else {
            int16_t abs_exponential = -1 * opts.exponential;
            num *= pow(10, abs_exponential + opts.precision);
            decimal_idx = pow(10, opts.precision);
        }
        if (errno == ERANGE) {
            throw std::runtime_error("Math error detected.");
        }
        while (num / divisor > 10) {
            // identify num width thus intializing digit parsing.
            //  the detected width (characterized by divisor and digit_count for natural width) is used for parsing, padding and truncating...
            divisor *= 10;
            if (divisor >= decimal_idx) {
                digit_count++;
            }
        }
        if (opts.width != UINT8_MAX && digit_count < opts.width) {
            // pad to specified width
            opts.width -= digit_count;
            while (opts.width > 0) {
                s += '0';
            }
        } else if (digit_count > opts.width) {
            // otherwise truncate
            start_index = digit_count - opts.width;
        }
        // digit parsing, place decimal point and truncate at precision.
        while (divisor > 0) {
            digit = (char)(num / divisor);
            if (i >= start_index) { // truncate
                if (precision_count >= 0) {
                    precision_count++;
                } else if (precision_count > opts.precision) {
                    break;
                }
                digit = (char)(num / divisor);
                if (digit <= 9) {
                    s += digit + '0';
                    if (divisor == decimal_idx) {
                        s += '.';
                        precision_count = 0;
                    }
                } else {
                    throw std::runtime_error("Invalid digit character detected.");
                }
            }
            num -= (digit * divisor);
            divisor /= 10;
            i++;
        }
        // append exponential string.
        if (opts.exponential != 0) {
            s += 'E';
            if (opts.exponential < 0) {
                s += '-';
                opts.exponential *= -1;
            } else {
                s += '+';
            }
            StringFormatOpts opts;
            s += numToStringSigned(opts.exponential, opts);
        }
        return s;
    }
}

#endif