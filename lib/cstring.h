#ifndef WYLESLIBS_CSTRING_H
#define WYLESLIBS_CSTRING_H

// TODO:
#include <string>
#include <stdbool.h>

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

static char hexToChar(std::string * buf) {
    char ret = 0x00;
    for (size_t i = 0; i < 2; i++) {
        char c = buf->at(i);
        if (isDigit(c)) {
            c = c - 0x30;
        } else if (isLowerHex(c)) {
            c = c - 0x61;
        } else if (isUpperHex(c)) {
            c = c - 0x41;
        } else {
            // TODO:
            //  freak out! exception... something...
        }
        ret = ret << 4 | c;
    }
    return ret;
}

#endif