#ifndef WYLESLIBS_STRING_UTILS_H
#define WYLESLIBS_STRING_UTILS_H

#include <math.h>
#include <string>
#include <stdbool.h>
#include <stdexcept>

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

static std::string NumToString(int64_t num, size_t base = 10, bool upper = true) {
    std::string s;
    size_t divisor = 1;
    if (base != 10 && base != 16) {
        throw std::runtime_error("Base not supported.");
    }
    if (num < 0) {
        s += '-';
        num *= -1;
    }
    if (base == 16) {
        s += "0x";
    }
    while (num / divisor > base) {
        divisor *= base;
    }
    while (divisor > 0) {
        char digit = (char)(num / divisor);
        if (digit <= 9) {
            s += digit + '0';
        } else if (digit < 0xF) {
            if (true == upper) {
                s += digit + 'A';
            } else {
                s += digit + 'a';
            }
        } else {
            throw std::runtime_error("Invalid digit character detected.");
        }
        num -= (digit * divisor);
        divisor /= base;
    }
    return s;
}

static std::string FloatToString(double num, uint8_t precision = 6, int16_t exponential = 0) {
    std::string s;
    if (num < 0) {
        s += '-';
    }
    int16_t precision_count = -1;
    size_t divisor = 1;
    size_t decimal_idx;
    // process exponential
    if (exponential == 0) {
        decimal_idx = pow(10, precision);
        num *= decimal_idx;
    } else if (exponential > 0) {
        size_t width_divisor = 1;
        size_t width = 1;
        while (num / width_divisor > 10) {
            width_divisor *= 10;
            width++;
        }
        if (static_cast<int64_t>(width) < exponential) {
            s += "0.";
            size_t pad_count = exponential - width;
            bool zeroed_result = false;
            if (precision > pad_count) {
                precision_count = pad_count + 1;
            } else {
                pad_count = precision;
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
            decimal_idx = pow(10, precision + exponential);
        }
        num *= pow(10, precision);
    } else {
        int16_t abs_exponential = -1 * exponential;
        num *= pow(10, abs_exponential + precision);
        decimal_idx = pow(10, precision);
    }
    if (errno == ERANGE) {
        throw std::runtime_error("Math error detected.");
    }
    // identify num width thus intializing digit parsing
    while (num / divisor > 10) {
        divisor *= 10;
    }
    // digit parsing, place decimal point and truncate at precision.
    while (divisor > 0) {
        if (precision_count >= 0) {
            precision_count++;
        } else if (precision_count > precision) {
            break;
        }
        char digit = (char)(num / divisor);
        if (digit <= 9) {
            s += digit + '0';
            if (divisor == decimal_idx) {
                s += '.';
                precision_count = 0;
            }
        } else {
            throw std::runtime_error("Invalid digit character detected.");
        }
        num -= (digit * divisor);
        divisor /= 10;
    }
    // append exponential string.
    if (exponential != 0) {
        s += 'E';
        if (exponential < 0) {
            s += '-';
            exponential *= -1;
        } else {
            s += '+';
        }
        s += NumToString(exponential);
    }
    return s;
}

#endif