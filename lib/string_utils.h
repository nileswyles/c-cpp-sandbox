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
            static constexpr uint8_t NO_SIGN = 0x0;
            static constexpr uint8_t SIGN_FOR_NEGATIVES = 0x1;
            static constexpr uint8_t SIGN_FOR_POSITIVES = 0x2;

            uint8_t base;
            uint8_t width;
            uint8_t precision;
            int16_t exponential;
            char exponential_designator;
            bool upper;
            uint8_t sign_mask;
            bool is_negative;

            StringFormatOpts(): base(10), width(UINT8_MAX), precision(6), exponential(0), exponential_designator('E'), upper(true), sign_mask(SIGN_FOR_NEGATIVES), is_negative(false) {}
    };

    static std::string numToString(uint64_t num, StringFormatOpts opts = {}) {
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
        size_t digit_count = 1;
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
                opts.width--;
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

    static std::string numToStringSigned(int64_t num, StringFormatOpts opts = {}) {
        if (num < 0) {
            num *= -1;
            opts.is_negative = true;
        }
        return numToString(static_cast<uint64_t>(num), opts);
    }

    static std::string floatToString(double num, StringFormatOpts opts = {}) {
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
        size_t digit_count = 1;
        size_t digit_count_before_decimal = 0;
        size_t start_index = 0;
        size_t decimal_idx;
        char digit;
        size_t i = 0;

        // process exponential
        if (opts.exponential == 0) {
            decimal_idx = pow(10, opts.precision);
            num *= decimal_idx;
        } else if (opts.exponential > 0) {
            decimal_idx = pow(10, opts.precision + opts.exponential);
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
                digit_count_before_decimal++;
            }
            digit_count++;
        }
        if (digit_count_before_decimal == 0) {
            // handle specific case where the user requests an exponential but doesn't number doesn't have enough digits to satisfy, in this situation we need to pad after the decimal point in addition to before.
            // natural pad
            if (opts.width != UINT8_MAX) {
                size_t natural_pad = opts.width;
                while (natural_pad > 0) {
                    s += '0';
                    natural_pad--;
                }
            } else {
                s += '0';
            }
            if (opts.precision == 0) {
                return s;
            }
            size_t digit_count_to_decimal_idx = 0;
            size_t pow_10 = 1;
            while (pow_10 != decimal_idx) {
                pow_10 *= 10;
                digit_count_to_decimal_idx++;
            }
            // decimal pad
            size_t decimal_pad = digit_count_to_decimal_idx - digit_count;
            if (opts.precision <= decimal_pad) {
                return s;
            }
            s += '.';
            precision_count = 0;
            while (decimal_pad > 0) {
                s += '0';
                decimal_pad--;
                precision_count++;
            }
        } else {
            if (opts.width != UINT8_MAX && digit_count_before_decimal < opts.width) {
                // pad to width
                opts.width -= digit_count_before_decimal;
                while (opts.width > 0) {
                    s += '0';
                    opts.width--;
                }
            } else if (digit_count_before_decimal > opts.width) {
                // otherwise truncate
                start_index = digit_count_before_decimal - opts.width;
            }
        }
        // digit parsing - place decimal point and trim left at width and right at precision.
        while (divisor > 0) {
            digit = (char)(num / divisor);
            if (i >= start_index) { // truncate natural

                if (precision_count >= opts.precision) { // truncate decimal
                    break;
                } else if (precision_count >= 0) {
                    precision_count++;
                }
                if (digit <= 9) {
                    s += digit + '0';
                    if (divisor == decimal_idx) {
                        if (opts.precision == 0) {
                            // if precision is zero, obviously don't continue after decimal
                            break;
                        }
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
            s += opts.exponential_designator;
            if (opts.exponential < 0) {
                s += '-';
                opts.exponential *= -1;
            } else {
                s += '+';
            }
            s += numToStringSigned(opts.exponential);
        }
        return s;
    }
}

#endif