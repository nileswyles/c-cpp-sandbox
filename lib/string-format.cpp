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

static Arg parsePositionalPercentFormat(va_list args, EStream& s) {
    std::string v;
    void * ptr = nullptr;
    char type;

    char c = s.get();
    SharedArray<uint8_t> f = s.readUntil(END_OF_FORMAT_STR);
    if (1 == f.size() && (c == 'b' || c == 'c' || c == 't' || c == 's')) { // these must be size 1
        if (c == 'b') {
            type = 'b';
            v = (true == (bool)va_arg(args, int)) ? "true" : "false";
        } else if (c == 't') {
            std::basic_istream<char> * ss = va_arg(args, std::basic_istream<char> *);
            type = 't';
            // odd that basic_ostream doesn't implement << for basic_istream
            while (true == ss->good()) {
                char c = ss->get();
                // just alphanumeric and special or no EOF?
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
    } else {
        char expanded_value[MAX_FORMAT_LENGTH];
        std::string fmt("%");
        fmt += c;
        if (1 < f.size()) {
            f.removeBack(); // remove END_OF_FORMAT_CHAR
            c = f.back(); // last char
            fmt += f.toString();
            std::cout << "format: " << fmt << std::endl;
        }
        if (c == 'd' || c == 'x' || c == 'X' || c == 'o') {
            // TODO: if no type safety then why this?
            // read up on va_arg type parameter... 
            int i = va_arg(args, int);
            ptr = (void *)new int(i);
            type = 'd';
            sprintf(expanded_value, fmt.c_str(), i);
            v = expanded_value;
        } else if (c == 'f' || c == 'e' || c == 'E') {
            double f = va_arg(args, double);
            ptr = (void *)new double(f);
            type = 'f';
            sprintf(expanded_value, fmt.c_str(), f);
            v = expanded_value;
        } else {
            std::stringstream ss;
            ss << "Invalid positional percent format specifier. Format: " << fmt;
            throw std::runtime_error(ss.str());
        }
    }
    return Arg(ptr, type, v);
}

static std::string parseDecimalFormatAndConvert() {


}

static std::string parseIndicatorPercentFormat(EStream& s, Arg arg) {
    std::string v;
    char start_char = s.get();

    char c = s.get();
    SharedArray<uint8_t> f = s.readUntil(END_OF_FORMAT_STR);
    if (1 == f.size() && (c == 'b' || c == 'c' || c == 't' || c == 's')) { // these must be size 1
        v = arg.expanded_value;
    } else {
        char expanded_value[MAX_FORMAT_LENGTH];
        std::string fmt("%");
        fmt += c;
        if (1 < f.size()) {
            f.removeBack(); // remove END_OF_FORMAT_CHAR
            c = f.back(); // last char
            fmt += f.toString();
            std::cout << "format: " << fmt << std::endl;
        }
        // TODO: implement my own formatting?
        //      pros:
        //          - potentially less stack memory usage from compile-time char buffer allocation.
        //          - safety
        //      cons:
        //          - work... i'm lazy...

        //      pros for keeping:
        //          - less work. I am lazy.
        //      cons for keeping:
        //          - potential user error (unlikely however)

        // maybe implement dec but not float? floats usually specifi
        if (c == 'd' || c == 'x' || c == 'X' || c == 'o') {
            sprintf(expanded_value, fmt.c_str(), *((int *)arg.ptr));
            v = expanded_value;
        } else if (c == 'f' || c == 'e' || c == 'E') {
            sprintf(expanded_value, fmt.c_str(), *((double *)arg.ptr));
            v = expanded_value;
        } else {
            std::stringstream ss;
            ss << "Invalid indicator percent format specifier. Format: " << fmt;
            throw std::runtime_error(ss.str());
        }
    }
    return v;
}

// TODO:
// escape characters...
//  {} and any required by printf? if any?
//  if don't need to escape characters for printf then maybe choose different start/end characters?
static void parsePositionalFormat(va_list args, EStream& s, std::stringstream& d, Array<Arg>& converted_args, bool& has_indicator_selection) {
    // i == {
    char c = s.get();
    Arg arg;

    // passthrough pointer/reference/indicator for second pass
    //  TODO: think about whether going back to main loop is a better implementation.
    if (true == isDigit(c)) {
        // it's an indicator type, reserve parsing for later.
        d.put(START_OF_FORMAT_CHAR);
        while(c != END_OF_FORMAT_CHAR) {
            d.put(c);
            c = s.get();
        }
        d.put(END_OF_FORMAT_CHAR);
        has_indicator_selection = true;
        return;
    } 
    // parse format if no indicator.
    if (c == END_OF_FORMAT_CHAR) {
        arg.expanded_value = va_arg(args, const char *);
        arg.type = 's';
    } else if (c == '%') {
        arg = parsePositionalPercentFormat(args, s);
    } else {
        std::stringstream ss;
        ss << "Invalid positional format. Detected the following character: " << c;
        throw std::runtime_error(ss.str());
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
        // parse number...
        double lol = 0;
        size_t dummy_count;
        s.readNatural(lol, dummy_count);
        selection = static_cast<size_t>(lol) - 1; // 0 - 1 == SIZE_MAX?
    } else {
        std::stringstream ss;
        ss << "Invalid indicator format. An indicator format must start with a number and detected the following character: " << c;
        throw std::runtime_error(ss.str());
    }
    // is pointer good?
    if (selection == SIZE_MAX || selection >= args.size()) {
        std::stringstream ss;
        ss << "Invalid Selection in indicator format: " << std::dec << selection;
        throw std::runtime_error(ss.str());
    } else {
        // use pointer value or parse format override.
        std::string v;
        Arg arg = args[selection];
        if (c == END_OF_FORMAT_CHAR) {
            v = arg.expanded_value;
        } else if (c == ',') {
            c = s.get();
            if (c == '%') { // else must be type specifier?
                v = parseIndicatorPercentFormat(s, arg);
            } else {
                std::stringstream ss;
                ss << "Invalid indicator format. The following character is expected to be %: " << c;
                throw std::runtime_error(ss.str());
            }
        } else {
            std::stringstream ss;
            ss << "Invalid indicator positional format. The following character is expected to be a comma: " << c;
            throw std::runtime_error(ss.str());
        }
        d.write(v.data(), v.size());
    }
    // } // should never see empty format block here
    // i == }
}

static void deleteArgs(Array<Arg>& args) {
    for (auto arg: args) {
        char c = arg.type;
        // but why?
        if (c == 'd' || c == 'x' || c == 'X' || c == 'o') {
            delete (int *)arg.ptr;
        } else if (c == 'f' || c == 'e' || c == 'E') {
            delete (double *)arg.ptr;
        } else {
            std::stringstream ss;
            ss << "Something went terribly wrong while deleting the data structure containing arguments for the indicator functionality.";
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