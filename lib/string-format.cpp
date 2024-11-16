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
#define START_OF_INDICATOR_FORMAT_CHAR '<'
#define INDICATOR_FORMAT_SEPARATOR ','

using namespace WylesLibs;

static std::string parseFloatFormatSpecifierAndConvert(EStream& s, void * value) {
    size_t precision = 6;
    char c = s.peek();
    if (true == isDigit(c)) {
        double lol = 0;
        size_t dummy_count = 0;
        s.readNatural(lol, dummy_count);
        precision = static_cast<size_t>(lol);
    }
    char format_type = s.get();
    int16_t exponential = 0;
    if (format_type != 'E' && format_type != 'e' && (format_type != 'f' || (format_type == 'f' && s.peek() != END_OF_FORMAT_CHAR))) {
        std::basic_stringstream<char> ss;
        ss << "Invalid format at: '" << format_type << s.peek() << "'";
        throw std::runtime_error(ss.str());
    } else if (format_type == 'E' || format_type == 'e') {
        c = s.get();
        if (c == '+' || c == '-') {
            if (c == '-') {
                exponential = -1;
            }
            c = s.peek();
            if (true == isDigit(c)) {
                double lol = 0;
                size_t dummy_count = 0;
                s.readNatural(lol, dummy_count);
                exponential *= static_cast<size_t>(lol);
            }
            if (s.peek() != END_OF_FORMAT_CHAR) {
                std::basic_stringstream<char> ss;
                ss << "Invalid float format. Expected '" << END_OF_FORMAT_CHAR << "' after the exponential field specifier.";
                throw std::runtime_error(ss.str());
            }
        } else {
            throw std::runtime_error("Invalid float format.");
        }
    }
    return FloatToString(*(double *)value, precision, exponential);
}

static Arg parsePositionalFormatSpecifier(va_list args, EStream& s) {
    std::string v;
    void * ptr = nullptr;
    char type = WYLESLIBS_STRING_FORMAT_DEFAULT_ARG_TYPE_CHAR;

    char c = s.get();
    char next = s.peek();
    if (next == END_OF_FORMAT_CHAR && (c == 'b' || c == 'c' || c == 't' || c == 's')) { // these must be size 1
        if (c == 'b') {
            v = (true == (bool)va_arg(args, int)) ? "true" : "false";
        } else if (c == 't') {
            std::basic_istream<char> * ss = va_arg(args, std::basic_istream<char> *);
            while (true == ss->good()) {
                char c = ss->get();
                if (c != 0xFF) {
                    v += c;
                }
            }
        } else if (c == 's') {
            v = va_arg(args, const char *);
        } else if (c == 'c') {
            char i = (char)va_arg(args, int);
            v += i;
        }
    // } else if (next == END_OF_FORMAT_CHAR && (c == 'd' || c == 'x' || c == 'X' || c == 'o')) {
    // TODO: no octal support for now
    } else if (next == END_OF_FORMAT_CHAR && (c == 'd' || c == 'x' || c == 'X')) {
        // TODO: if no type safety then why this?
        // read up on va_arg type parameter... 
        int i = va_arg(args, int);
        ptr = (void *)new int(i);
        type = 'd';
        v = NumToString(*(int *)ptr, c == 'd' ? 10 : 16, c == 'X');
    } else {
        s.unget(); // place c back in stream...

        double i = va_arg(args, double);
        ptr = (void *)new double(i);
        type = 'f';
        v = parseFloatFormatSpecifierAndConvert(s, ptr);
    }
    s.get(); // make sure to consume END_OF_FORMAT_CHAR
    return Arg(ptr, type, v);
}

static std::string parseReferenceFormatSpecifier(EStream& s, Arg& arg) {
    std::string v;
    char c = s.get();
    char next = s.peek();
    if (next == END_OF_FORMAT_CHAR && (c == 'b' || c == 'c' || c == 't' || c == 's')) { // these must be size 1
        v = arg.expanded_value;
    } else if (next == END_OF_FORMAT_CHAR && (c == 'd' || c == 'x' || c == 'X' || c == 'o')) {
        v = arg.expanded_value;
    } else {
        s.unget(); // place c back in stream...

        if (arg.ptr == nullptr || arg.type != 'f') {
            std::basic_stringstream<char> ss;
            ss << "Invalid arg selected. Expected non-null pointer and arg.type = 'f'. Is pointer null? " << (int)(arg.ptr == nullptr) << ". Arg type: '" << arg.type << "'.";
            throw std::runtime_error(ss.str());
        }
        v = parseFloatFormatSpecifierAndConvert(s, arg.ptr);
    }
    s.get(); // make sure to consume END_OF_FORMAT_CHAR
    return v;
}

static void parsePositionalFormat(va_list args, EStream& s, std::basic_stringstream<char>& d, Array<Arg>& converted_args, bool& has_reference_selection) {
    // i == {
    char c = s.peek();
    Arg arg;
    if (c == START_OF_INDICATOR_FORMAT_CHAR) {
        // it's a reference type, reserve parsing for later - don't fail fast, I guess.
        d.put(START_OF_FORMAT_CHAR);
        has_reference_selection = true;
        return;
    } 
    // parse format if not a reference type.
    if (s.peek() == END_OF_FORMAT_CHAR) {
        s.get();
        arg.expanded_value = va_arg(args, const char *);
    } else {
        arg = parsePositionalFormatSpecifier(args, s);
    }
    d.write(arg.expanded_value.data(), arg.expanded_value.size());
    converted_args.append(arg);
    // i == }
}

static void parseReferenceFormat(Array<Arg>& args, EStream& s, std::basic_stringstream<char>& d) {
    // i == {
    size_t selection = SIZE_MAX;
    char c = s.get(); // < required
    if (c == START_OF_INDICATOR_FORMAT_CHAR) {
        // parse reference
        c = s.peek(); // first digit required
        if (true == isDigit(c)) {
           double lol = 0;
           size_t dummy_count = 0;
           s.readNatural(lol, dummy_count);
           selection = static_cast<size_t>(lol) - 1; // 0 - 1 == SIZE_MAX?
           // consumed all digits, at non-digit
           //    std::cout << "Selected positional format: " << selection << std::endl;
        } else {
           std::basic_stringstream<char> ss;
           ss << "Invalid reference format. A reference format must start with a '" << START_OF_INDICATOR_FORMAT_CHAR << "' and number - the following character was detected: '" << c << "'";
           throw std::runtime_error(ss.str());
       }
    } else {
        std::basic_stringstream<char> ss;
        ss << "Invalid reference format. A reference format must start with a '" << START_OF_INDICATOR_FORMAT_CHAR << "' and the following character was detected: '" << c << "'";
        throw std::runtime_error(ss.str());
    }
    if (selection == SIZE_MAX || selection >= args.size()) {
        // is reference valid?
        std::basic_stringstream<char> ss;
        ss << "Invalid selection in reference format: " << std::dec << selection;
        throw std::runtime_error(ss.str());
    } else {
        c = s.get(); // } or , required and consumed.
        std::string v;
        Arg arg = args[selection];
        // use referenced value or parse format override.
        if (c == END_OF_FORMAT_CHAR) {
            v = arg.expanded_value;
        } else if (c == INDICATOR_FORMAT_SEPARATOR) {
            // # generalizations sometimes?
            v = parseReferenceFormatSpecifier(s, arg);
        } else {
            std::basic_stringstream<char> ss;
            ss << "Invalid reference format. The following character is expected to be a comma: " << c;
            throw std::runtime_error(ss.str());
        }
        d.write(v.data(), v.size());
    }
    // i == }
}

static void deleteArgs(Array<Arg>& args) {
    for (auto arg: args) {
        char c = arg.type;
        if (c == 'd') {
            delete (int *)arg.ptr;
        } else if (c == 'f') {
            delete (double *)arg.ptr;
        } else if (c != WYLESLIBS_STRING_FORMAT_DEFAULT_ARG_TYPE_CHAR && arg.ptr != nullptr) {
            std::basic_stringstream<char> ss;
            ss << "Invalid type detected in reference args structure - failed to delete. Type identifier: " << c;
            throw std::runtime_error(ss.str());
        }
    }
}

extern std::string WylesLibs::format(std::string format, ...) {
    va_list args;
    va_start(args, format);
    // if reach end of format and still va_arg, then do what? can just ignore if just positional but might be an issue with reference stuff..
    bool escaped = false;
    bool has_reference_selection = false;
    std::basic_stringstream<char> format_out;
    Array<Arg> parsed_args;
    EStream s((uint8_t *)format.data(), format.size());
    char c;
    // options:
    //  reference previous positional and indicator?
    //      requires 1 pass.
    //      then no override. unless it takes from top-level? lol
    //      yeah, definetly...
    //  reference previous positional only.
    //      requires 1 pass.
    //      I think it's useful to not have to repeat the variable if of same data... lol... so override might be necessary...
    //  reference any positional format. // this was the original goal.
    //      requires 2 passes.
    //      I think it's useful to not have to repeat the variable if of same data... lol... so override might be necessary...
    //      might also be useful to reference any positional format. not as bad if just referencing positional formats.
    //  reference any positional or indicator format.
    //      requires 2 passes.
    //      then no override. unless it takes from top-level? lol too confusing too much compression??
    //      yeah, definetly...
    //      might also be useful to reference any positional format but now even more complication... 

    // alright, so I was right. Like always... LMAO jk
    while (true == s.good()) {
        c = s.get();
        if (c == '\\') {
            // escaping...
            char next = s.peek();
            if (c == '{' || c == '}') {
                s.get();
                format_out.put(c);
                format_out.put(next);
                continue;
                // make sure to skip parsing but keep escape sequence.
            }
        } 
        if (c == '{') {
            parsePositionalFormat(args, s, format_out, parsed_args, has_reference_selection);
        } else {
            format_out.put(c);
        }
    }
    va_end(args);

    if (false == escaped && false == has_reference_selection) {
        deleteArgs(parsed_args);
        return format_out.str();
    } else {
        std::cout << "Format out: " << format_out.str() << std::endl;
        std::basic_stringstream<char> format_out_2;
        s = EStream((uint8_t *)format_out.str().data(), format_out.str().size());
        while (true == s.good()) {
            c = s.get();
            if (c == '\\') {
                // escaping...
                char next = s.peek();
                if (c == '{' || c == '}') {
                    s.get();
                    format_out_2.put(next);
                    continue;
                    // make sure to skip parsing but undo escaping sequence.
                }
            } 
            if (c == '{') {
                parseReferenceFormat(parsed_args, s, format_out_2);
            } else {
                format_out_2.put(c);
            }
        }
        deleteArgs(parsed_args);
        return format_out_2.str();
    }
}

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