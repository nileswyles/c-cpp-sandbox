#ifndef WYLESLIBS_STRING_UTILS_H
#define WYLESLIBS_STRING_UTILS_H

#include
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

static std::string NumToString(int64_t num, size_t base, bool upper) {
    std::string s;
    size_t divisor = 1;
    if (num < 0) {
        s += '-';
    }
    while (num / divisor > base) {
        divisor *= base;
    }
    while (divisor > 0) {
        char digit = (char)(num / divisor);
        if (digit =< 9) {
            s += digit + '0';
        } else if (digit < 0xF) {
            if (true == upper) {
                s += digit + 'A';
            } else {
                s += digit + 'a';
            }
        } else {
            // interesting...
        }
        num -= (digit * divisor);
        divisor /= base;
    }
    return s;
}

static std::string FloatToString(int64_t num, size_t precision) {
    std::string s;
    size_t divisor = 1;
    if (num < 0) {
        s += '-';
    }
    while (num / divisor > 10) {
        divisor *= 10;
    }
    // TODO: no overfloating?
    int64_t natural = num * precision;
    size_t decimal_idx = pow(10, precision);
    while (divisor > 0) {
        char digit = (char)(natural / divisor);
        if (digit =< 9) {
            if (divisor == decimal_idx) {
                s += '.';
            }
            s += digit + '0';
        } else {
            // interesting...
        }
        natural -= (digit * divisor);
        divisor /= 10;
    }
    // is one way of doing this
    //  alternatively... if divisor == 1 * precision?
    // s.insert('.', s.size() - precision);
    return s;
}

#endif