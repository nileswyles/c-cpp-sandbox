#ifndef WYLESLIB_PARSER_COMMON_H
#define WYLESLIB_PARSER_COMMON_H

#include "string_utils.h"
#include "reader/reader.h"

namespace WylesLib::Parser {
// TODO:
//  move to reader?
// Let's define that parse function's start index is first index of token and end index is last index of token + 1.
static void parseDecimal(Reader * r, double& value, size_t& digit_count) {
    double decimal_divisor = 10;
    char c = r->peekByte();
    while (isDigit(c)) {
        r->readByte();
        value += (c - 0x30) / decimal_divisor;
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        decimal_divisor *= 10;
        c = r->peekByte();
        if (++digit_count > FLT_MAX_MIN_DIGITS) {
            std::string msg = "parseDecimal: Exceeded decimal digit limit.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    }
}

static void parseNatural(Reader * r, double& value, size_t& digit_count) {
    char c = r->peekByte();
    while (isDigit(c)) {
        r->readByte();
        value = (value * 10) + (c - 0x30); 
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        c = r->peekByte();
        if (++digit_count > FLT_MAX_MIN_DIGITS) {
            std::string msg = "parseNatural: Exceeded natural digit limit.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    }
}
}
#endif