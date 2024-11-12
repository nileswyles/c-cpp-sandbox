#include "string-format.h"

#include "datastructures/array.h"
#include "string_utils.h"
#include "estream/estream.h"

#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_FORMAT_LENGTH 64
#define START_OF_FORMAT_CHAR '{'
#define END_OF_FORMAT_CHAR '}'
#define END_OF_FORMAT_STR "}"

using namespace WylesLibs;

static std::string parseFloatFormatSpecifierAndConvert(EStream& s, Arg& arg) {
    size_t precision = 6;
    char c = s.peek();
    if (true == isDigit(c)) {
        double lol = 0;
        size_t dummy_count;
        s.readNatural(lol, dummy_count);
        precision = static_cast<size_t>(lol);
    }
    char format_type = s.get();
    int16_t exponential = 1;
    if (format_type != 'E' && format_type != 'e' && format_type != 'f') {
        throw std::runtime_error("Invalid float format.");
    } else if (format_type == 'E' || format_type == 'e') {
        c = s.get();
        if (c == '+' || c == '-') {
            if (c == '-') {
                exponential *= -1;
            }
            c = s.peek();
            if (true == isDigit(c)) {
                double lol = 0;
                size_t dummy_count;
                s.readNatural(lol, dummy_count);
                exponential *= static_cast<size_t>(lol);
            }
        } else {
            throw std::runtime_error("Invalid float format.");
        }
    }
    return FloatToString(*(double *)arg.ptr, precision, exponential);
}

static Arg parsePositionalFormatSpecifier(va_list args, EStream& s) {
    std::string v;
    void * ptr = nullptr;
    char type;

    char c = s.get();
    char next = s.peek();
    if (next == END_OF_FORMAT_CHAR && (c == 'b' || c == 'c' || c == 't' || c == 's')) { // these must be size 1
        if (c == 'b') {
            type = 'b';
            v = (true == (bool)va_arg(args, int)) ? "true" : "false";
        } else if (c == 't') {
            std::basic_istream<char> * ss = va_arg(args, std::basic_istream<char> *);
            type = 't';
            while (true == ss->good()) {
                char c = ss->get();
                if (c != 0xFF) {
                    v += c;
                }
            }
        } else if (c == 's') {
            v = va_arg(args, const char *);
            type = 's';
        } else if (c == 'c') {
            char i = (char)va_arg(args, int);
            type = 'c';
            v += i;
        }
    } else if (next == END_OF_FORMAT_CHAR && (c == 'd' || c == 'x' || c == 'X' || c == 'o')) {
        // TODO: if no type safety then why this?
        // read up on va_arg type parameter... 
        int i = va_arg(args, int);
        ptr = (void *)new int(i);
        type = 'd';
        v = NumToString(*(int *)ptr, 10);
    } else {
        int i = va_arg(args, int);
        ptr = (void *)new int(i);
        type = 'f';
        Arg whatever(ptr, type, "");
        v = parseFloatFormatSpecifierAndConvert(s, whatever);
    }
    return Arg(ptr, type, v);
}

static std::string parseIndicatorFormatSpecifier(EStream& s, Arg arg) {
    std::string v;
    char c = s.get();
    char next = s.peek();
    if (next == END_OF_FORMAT_CHAR && (c == 'b' || c == 'c' || c == 't' || c == 's')) { // these must be size 1
        v = arg.expanded_value;
    } else if (next == END_OF_FORMAT_CHAR && (c == 'd' || c == 'x' || c == 'X' || c == 'o')) {
        v = NumToString(*(int *)arg.ptr, 10);
    } else {
        v = parseFloatFormatSpecifierAndConvert(s, arg);
    }
    return v;
}

static void parsePositionalFormat(va_list args, EStream& s, std::stringstream& d, Array<Arg>& converted_args, bool& has_indicator_selection) {
    // i == {
    char c = s.get();
    Arg arg;

    // passthrough pointer/reference/indicator for second pass
    if (true == isDigit(c)) {
        // it's an indicator type, reserve parsing for later.
        d.put(START_OF_FORMAT_CHAR);
        d.put(c);

        has_indicator_selection = true;
        return;
    } 
    // parse format if no indicator.
    if (c == END_OF_FORMAT_CHAR) {
        arg.expanded_value = va_arg(args, const char *);
        arg.type = 's';
    } else {
        arg = parsePositionalFormatSpecifier(args, s);
    }
    d.write(arg.expanded_value.data(), arg.expanded_value.size());
    converted_args.append(arg);
    // i == }
}

static void parseIndicatorFormat(Array<Arg>& args, EStream& s, std::stringstream& d) {
    // i == {
    size_t selection = SIZE_MAX;
    char c = s.peek();
    // parse pointer/reference/indicator
    if (true == isDigit(c)) {
        double lol = 0;
        size_t dummy_count;
        s.readNatural(lol, dummy_count);
        selection = static_cast<size_t>(lol) - 1; // 0 - 1 == SIZE_MAX?
    } else {
        std::stringstream ss;
        ss << "Invalid indicator format. An indicator format must start with a number and the following character was detected: " << c;
        throw std::runtime_error(ss.str());
    }
    // is pointer good?
    if (selection == SIZE_MAX || selection >= args.size()) {
        std::stringstream ss;
        ss << "Invalid selection in indicator format: " << std::dec << selection;
        throw std::runtime_error(ss.str());
    } else {
        // use pointer value or parse format override.
        std::string v;
        Arg arg = args[selection];
        if (c == END_OF_FORMAT_CHAR) {
            v = arg.expanded_value;
        } else if (c == ',') {
            v = parseIndicatorFormatSpecifier(s, arg);
        } else {
            std::stringstream ss;
            ss << "Invalid indicator format. The following character is expected to be a comma: " << c;
            throw std::runtime_error(ss.str());
        }
        d.write(v.data(), v.size());
    }
    // i == }
}

static void deleteArgs(Array<Arg>& args) {
    for (auto arg: args) {
        char c = arg.type;
        if (c == 'd' || c == 'x' || c == 'X' || c == 'o') {
            delete (int *)arg.ptr;
        } else if (c == 'f' || c == 'e' || c == 'E') {
            delete (double *)arg.ptr;
        } else {
            std::stringstream ss;
            ss << "Invalid type detected in indicator args structure - failed to delete.";
            throw std::runtime_error(ss.str());
        }
    }
}

extern std::string WylesLibs::format(std::string format, ...) {
    va_list args;
    va_start(args, format);
    // if reach end of format and still va_arg, then do what? can just ignore if just positional but might be an issue with indicator stuff..
    bool has_indicator_selection = false;
    std::stringstream d;
    Array<Arg> parsed_args;
    EStream s((uint8_t *)format.data(), format.size());
    char c = s.get();
    while (true == s.good()) {
        s.get();
        if (c == '\\') {
            // escaping...
            char next = s.peek();
            if (c == '{' || c == '}') {
                s.get();
                d.put(c);
                d.put(next);
                continue;
                // make sure to skip parsing but keep escape sequence.
            }
        } 
        if (c == '{') {
            parsePositionalFormat(args, s, d, parsed_args, has_indicator_selection);
        } else {
            d.put(c);
        }
        c = s.peek();
    }
    va_end(args);

    if (false == has_indicator_selection) {
        deleteArgs(parsed_args);
        return d.str();
    } else {
        std::stringstream indicator_d;
        EStream s_w_parsing((uint8_t *)d.str().data(), d.str().size());
        while (true == s_w_parsing.good()) {
            s_w_parsing.get();
            if (c == '\\') {
                // escaping...
                char next = s.peek();
                if (c == '{' || c == '}') {
                    s.get();
                    d.put(next);
                    continue;
                    // make sure to skip parsing but undo escaping sequence.
                }
            } 
            if (c == '{') {
                parseIndicatorFormat(parsed_args, s_w_parsing, indicator_d);
            } else {
                d.put(c);
            }
            c = s_w_parsing.peek();
        }
        deleteArgs(parsed_args);
        return indicator_d.str();
    }
}

// LMAO
#define MAX_FORMATTED_STRING_SIZE 1024
extern std::string WylesLibs::ssprintf(std::string format, ...) {
// static std::string format(std::string format, va_list args) {
    va_list args;
    va_start(args, format);

    if (format.size() * 1.25 > MAX_FORMATTED_STRING_SIZE) {
        throw std::runtime_error("lol");
    }
    char str[1024];
    sprintf(str, format.c_str(), args);

    return std::string(str);
}