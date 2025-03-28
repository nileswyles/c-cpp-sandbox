#include "string_format.h"

#include "datastructures/array.h"
#include "string_utils.h"
#include "estream/byteestream.h"

#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include "msc_crt_secure.h"

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_STRING_FORMAT
#define LOGGER_LEVEL_STRING_FORMAT GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_STRING_FORMAT
#define LOGGER_STRING_FORMAT 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_STRING_FORMAT

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_STRING_FORMAT
#include "logger.h"

#define MAX_FORMAT_LENGTH 64
#define START_OF_FORMAT_CHAR '{'
#define END_OF_FORMAT_CHAR '}'
#define END_OF_FORMAT_STR "}"
#define START_OF_INDICATOR_FORMAT_CHAR '<'
#define INDICATOR_FORMAT_SEPARATOR ','

using namespace WylesLibs;

static std::string parseFloatFormat(ByteEStream& s, void * value, StringFormatOpts& opts);
static void parseStringCaseModifier(std::string value, ByteEStream& s, Arg& arg);
static void parsePositionalNumberModifier(va_list args, ByteEStream& s, StringFormatOpts& opts, Arg& arg);
static void parseReferencedNumberModifier(ByteEStream& s, StringFormatOpts& opts, Arg& arg);
static void parsePositionalModifiableFormat(va_list args, ByteEStream& s, StringFormatOpts& opts, Arg& arg);
static void parseReferencedModifiableFormat(ByteEStream& s, StringFormatOpts& opts, Arg& arg);
static void parsePositionalFormat(va_list args, ByteEStream& s, Arg& arg);
static void parseReferencedFormatOverride(ByteEStream& s, Arg& arg);
static void parseFormat(va_list args, ByteEStream& s, std::basic_stringstream<char>& d, Array<Arg>& converted_args, bool& has_reference_selection);
static void parseReferenceFormat(Array<Arg>& args, ByteEStream& s, std::basic_stringstream<char>& d);
static void deleteArgs(Array<Arg>& args);

static std::string parseFloatFormat(ByteEStream& s, void * value, StringFormatOpts& opts) {
    char format_type = s.get();

    if (format_type == 'E' || format_type == 'e') {
        opts.exponential_designator = format_type;
        char c = s.get();
        if (c == '+' || c == '-') {
            if (c == '-') {
                opts.exponential = -1;
            }
            c = s.peek();
            if (true == isDigit(c)) {
                // if you need more than +-128 then you have problems... behavior is undefined.
                opts.exponential *= static_cast<int8_t>(std::get<0>(s.readNatural()));
            }
            if (s.peek() != END_OF_FORMAT_CHAR) {
                std::basic_stringstream<char> ss;
                ss << "Invalid float format. Expected '" << END_OF_FORMAT_CHAR << "' after the exponential field specifier.";
                loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
                throw std::runtime_error(ss.str());
            }
        } else {
            std::string msg("Invalid float format.");
            loggerPrintf(LOGGER_INFO, "Exception: %s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    } else if (format_type != 'f') {
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
        loggerPrintf(LOGGER_DEBUG, "Stream buffer dump: \n'%s'\n", s.stream_log.c_str());
#endif
        std::basic_stringstream<char> ss;
        ss << "Invalid float format with format type: '" << format_type << "'";
        loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
        throw std::runtime_error(ss.str());

    }
    return floatToString(*(double *)value, opts);
}

static void parseStringCaseModifier(std::string value, ByteEStream& s, Arg& arg) {
    s.unget();
    char c = s.get();
    char next = s.get();
    char nextnext = s.peek();
    if (c == 's' && next == 'l' && nextnext == END_OF_FORMAT_CHAR) {
        for (auto c: value) {
            printf("%c %s\n", c, arg.expanded_value.c_str());
            if ('A' <= c && c >= 'Z') {
                arg.expanded_value.push_back(c + ' ');
            }
        }
    } else if (c == 's' && next == 'u' && nextnext == END_OF_FORMAT_CHAR) {
        for (auto c: value) {
            if ('a' <= c && c >= 'z') {
                arg.expanded_value.push_back(c - ' ');
            }
        }
    } else {
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
        loggerPrintf(LOGGER_DEBUG, "Stream buffer dump: \n'%s'\n", s.stream_log.c_str())
#endif
        std::basic_stringstream<char> ss;
        ss << "Invalid 'string with case modifier' format at: '" << c << next << nextnext << "'";
        loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
        throw std::runtime_error(ss.str());
    }
}

static void parsePositionalNumberModifier(va_list args, ByteEStream& s, StringFormatOpts& opts, Arg& arg) {
    uint64_t num = std::get<0>(s.readNatural());
    char next = s.peek();
    if (next == 'f' || next == 'e' || next == 'E') {
        opts.precision = static_cast<uint8_t>(num);
        parsePositionalModifiableFormat(args, s, opts, arg);
    } else {
        opts.width = static_cast<uint8_t>(num);
        if (next == '.') {
            // width
            // consume, '.' and proceed to parsing precision
            s.get();
            parsePositionalNumberModifier(args, s, opts, arg);
        } else {
            parsePositionalModifiableFormat(args, s, opts, arg);
        }
    }
}

static void parseReferencedNumberModifier(ByteEStream& s, StringFormatOpts& opts, Arg& arg) {
    uint64_t num = std::get<0>(s.readNatural());
    char next = s.peek();
    if (next == 'f' || next == 'e' || next == 'E') {
        opts.precision = static_cast<uint8_t>(num);
        parseReferencedModifiableFormat(s, opts, arg);
    } else {
        opts.width = static_cast<uint8_t>(num);
        if (next == '.') {
            // width
            // consume, '.' and proceed to parsing precision
            s.get();
            parseReferencedNumberModifier(s, opts, arg);
        } else {
            parseReferencedModifiableFormat(s, opts, arg);
        }
    }
}

static void parsePositionalModifiableFormat(va_list args, ByteEStream& s, StringFormatOpts& opts, Arg& arg) {
    char prev = s.peek();
    char c = s.get();
    char next = s.peek();
    arg.type = c;
    if (next == END_OF_FORMAT_CHAR && (c == 'd')) {
        int64_t x = va_arg(args, int64_t);
        arg.expanded_value = numToStringSigned(x, opts);
        arg.ptr = (void *)new int64_t(x);
    } else if (next == END_OF_FORMAT_CHAR && (c == 'u' || c == 'x' || c == 'X')) {
        uint64_t x = va_arg(args, uint64_t);
        opts.base = c == 'u' ? 10 : 16;
        opts.upper = c == 'X';
        arg.expanded_value = numToString(x, opts);
        arg.ptr = (void *)new uint64_t(x);
    } else if ((next == END_OF_FORMAT_CHAR && c == 'f') || c == 'E' || c == 'e') {
        s.unget(); // place c back in stream...
 
        double x = va_arg(args, double);
        arg.ptr = (void *)new double(x);
        arg.expanded_value = parseFloatFormat(s, arg.ptr, opts);
        arg.type = 'f';
    } else if (true == isDigit(c)) {
        s.unget();

        parsePositionalNumberModifier(args, s, opts, arg);
    } else {
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
        loggerPrintf(LOGGER_DEBUG, "Stream buffer dump: \n'%s'\n", s.stream_log.c_str())
#endif
        std::basic_stringstream<char> ss;
        ss << "Invalid positional format at: '" << prev << c << next << "'";
        loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
        throw std::runtime_error(ss.str());
    }
}

static void parseReferencedModifiableFormat(ByteEStream& s, StringFormatOpts& opts, Arg& arg) {
    char prev = s.peek();
    char c = s.get();
    char next = s.peek();
    if (next == END_OF_FORMAT_CHAR && (c == 'x' || c == 'X' || c == 'u')) {
        if (arg.ptr == nullptr || (arg.type != 'u' && arg.type != 'x' && arg.type != 'X')) {
            std::basic_stringstream<char> ss;
            ss << "Invalid arg reference. Expected non-null pointer and arg.type = 'u', 'x' or 'X'. Is pointer null? " << (int)(arg.ptr == nullptr) << ". Arg type: '" << arg.type << "'.";
            loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
            throw std::runtime_error(ss.str());
        }
        opts.base = c == 'u' ? 10 : 16;
        opts.upper = c == 'X';
        arg.expanded_value = numToString(*(uint64_t *)arg.ptr, opts);
    } else if (next == END_OF_FORMAT_CHAR && c == 'd') {
        if (arg.ptr == nullptr || arg.type != 'd') {
            std::basic_stringstream<char> ss;
            ss << "Invalid arg reference. Expected non-null pointer and arg.type = 'd'. Is pointer null? " << (int)(arg.ptr == nullptr) << ". Arg type: '" << arg.type << "'.";
            loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
            throw std::runtime_error(ss.str());
        }
        arg.expanded_value = numToStringSigned(*(int64_t *)arg.ptr, opts);
    } else if ((next == END_OF_FORMAT_CHAR && c == 'f') || c == 'E' || c == 'e') {
        if (arg.ptr == nullptr || arg.type != 'f') {
            std::basic_stringstream<char> ss;
            ss << "Invalid arg reference. Expected non-null pointer and arg.type = 'f'. Is pointer null? " << (int)(arg.ptr == nullptr) << ". Arg type: '" << arg.type << "'.";
            loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
            throw std::runtime_error(ss.str());
        }
        s.unget(); // place c back in stream...
 
        arg.expanded_value = parseFloatFormat(s, arg.ptr, opts);
    } else if (true == isDigit(c)) {
        s.unget();

        parseReferencedNumberModifier(s, opts, arg);
    } else {
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
        loggerPrintf(LOGGER_DEBUG, "Stream buffer dump: \n'%s'\n", s.stream_log.c_str())
#endif
        std::basic_stringstream<char> ss;
        ss << "Invalid referenced format at: '" << prev << c << next << "'";
        loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
        throw std::runtime_error(ss.str());
    }
}

// TODO: might be cool to implement this as a readUntil task lol... whatever..
static void parsePositionalFormat(va_list args, ByteEStream& s, Arg& arg) {
    StringFormatOpts opts;

    char prev = s.peek();
    char c = s.get();
    char next = s.peek();
    arg.type = c;
    if (next == END_OF_FORMAT_CHAR && c == 'b') {
        arg.expanded_value = (true == (bool)va_arg(args, int)) ? "true" : "false";
    } else if (next == END_OF_FORMAT_CHAR && c == 't') {
        std::basic_istream<char> * ss = va_arg(args, std::basic_istream<char> *);
        while (true == ss->good()) {
            char c = ss->get();
            // TODO: this doesn't sit right with me.
            if (static_cast<uint64_t>(c) != UINT64_MAX) {
                arg.expanded_value += c;
            }
        }
    } else if (next == END_OF_FORMAT_CHAR && c == 'c') {
        arg.expanded_value += (char)va_arg(args, int);
    } else if (next == END_OF_FORMAT_CHAR && c == 's') {
        arg.expanded_value = std::string(va_arg(args, const char *));
        arg.ptr = (void *)new std::string(arg.expanded_value);
    } else if (c == 's' && (next == 'l' || next == 'u')) {
        std::string value = std::string(va_arg(args, const char *));
        arg.ptr = (void *)new std::string(value);
        parseStringCaseModifier(value, s, arg);
    } else if (c == '+') {
        opts.sign_mask = StringFormatOpts::SIGN_FOR_POSITIVES | StringFormatOpts::SIGN_FOR_NEGATIVES;
        parsePositionalModifiableFormat(args, s, opts, arg);
    } else if (c == '-') {
        opts.sign_mask = StringFormatOpts::NO_SIGN;
        parsePositionalModifiableFormat(args, s, opts, arg);
    } else if (true == isDigit(c)) {
        s.unget();

        parsePositionalNumberModifier(args, s, opts, arg);
    } else {
        s.unget();

        parsePositionalModifiableFormat(args, s, opts, arg);
    }
    s.get(); // consume END_OF_FORMAT_CHAR
}

static void parseReferencedFormatOverride(ByteEStream& s, Arg& arg) {
    StringFormatOpts opts;

    char prev = s.peek();
    char c = s.get();
    char next = s.peek();
    if (next == END_OF_FORMAT_CHAR && (c == 'b' || c == 'c' || c == 't' || c == 's')) {
    } else if (c == 's' && (next == 'l' || next == 'u')) {
        if (arg.ptr == nullptr || arg.type != 's') {
            std::basic_stringstream<char> ss;
            ss << "Invalid arg reference. Expected non-null pointer and arg.type = 's'. Is pointer null? " << (int)(arg.ptr == nullptr) << ". Arg type: '" << arg.type << "'.";
            loggerPrintf(LOGGER_INFO, "Exception: %s\n", ss.str().c_str());
            throw std::runtime_error(ss.str());
        }
        std::string value = *(std::string *)arg.ptr;
        parseStringCaseModifier(value, s, arg);
    } else if (c == '+') {
        opts.sign_mask = StringFormatOpts::SIGN_FOR_POSITIVES | StringFormatOpts::SIGN_FOR_NEGATIVES;
        parseReferencedModifiableFormat(s, opts, arg);
    } else if (c == '-') {
        opts.sign_mask = StringFormatOpts::NO_SIGN;
        parseReferencedModifiableFormat(s, opts, arg);
    } else if (true == isDigit(c)) {
        s.unget();

        parseReferencedNumberModifier(s, opts, arg);
    } else {
        s.unget();

        parseReferencedModifiableFormat(s, opts, arg);
    }
    s.get(); // consume END_OF_FORMAT_CHAR
}

static void parseFormat(va_list args, ByteEStream& s, std::basic_stringstream<char>& d, Array<Arg>& converted_args, bool& has_reference_selection) {
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
        arg.expanded_value = std::string(va_arg(args, const char *));
    } else {
        parsePositionalFormat(args, s, arg);
    }
    d.write(arg.expanded_value.data(), arg.expanded_value.size());
    converted_args.append(arg);
    // i == }
}

static void parseReferenceFormat(Array<Arg>& args, ByteEStream& s, std::basic_stringstream<char>& d) {
    // i == {
    size_t selection = SIZE_MAX;
    char c = s.get(); // < required
    if (c != START_OF_INDICATOR_FORMAT_CHAR) {
        std::basic_stringstream<char> ss;
        ss << "Invalid reference format. A reference format must start with a '" << START_OF_INDICATOR_FORMAT_CHAR << "' and the following character was detected: '" << c << "'";
        throw std::runtime_error(ss.str());
    } else {
        // parse reference
        c = s.peek(); // first digit required
        if (false == isDigit(c)) {
           std::basic_stringstream<char> ss;
           ss << "Invalid reference format. A reference format must start with a '" << START_OF_INDICATOR_FORMAT_CHAR << "' and number - the following character was detected: '" << c << "'";
           throw std::runtime_error(ss.str());
        } else {
            selection = static_cast<size_t>(std::get<0>(s.readNatural())) - 1; // 0 - 1 == SIZE_MAX?
            // consumed all digits, at non-digit
       }
    }
    if (selection == SIZE_MAX || selection >= args.size()) {
        // is reference valid?
        std::basic_stringstream<char> ss;
        ss << "Invalid selection in reference format: " << std::dec << selection;
        throw std::runtime_error(ss.str());
    } else {
        c = s.get(); // } or , required and consumed.
        Arg arg = args[selection]; // ! IMPORTANT - since not Arg&, any modifications to arg here after don't affect the args stored in the array.
        // use referenced value or parse format override.
        if (c == INDICATOR_FORMAT_SEPARATOR) {
            // # generalizations sometimes?
            parseReferencedFormatOverride(s, arg);
        } else if (c != END_OF_FORMAT_CHAR){
            std::basic_stringstream<char> ss;
            ss << "Invalid reference format. The following character is expected to be a comma: " << c;
            throw std::runtime_error(ss.str());
        }
        d.write(arg.expanded_value.data(), arg.expanded_value.size());
    }
    // i == }
}

static void deleteArgs(Array<Arg>& args) {
    for (auto arg: args) {
        char c = arg.type;
        if (c == 'u' || c == 'X' || c == 'x') {
            delete (uint64_t *)arg.ptr;
        } else if (c == 'd') {
            delete (int64_t *)arg.ptr;
        } else if (c == 'f') {
            delete (double *)arg.ptr;
        } else if (c == 's') {
            delete (std::string *)arg.ptr;
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
    bool escaped = false;
    bool has_reference_selection = false;
    std::basic_stringstream<char> format_out;
    Array<Arg> parsed_args;
    ByteEStream s((uint8_t *)format.data(), format.size());
    char c;
    while (true == s.good()) {
        c = s.get();
        if (c == '\\') {
            // escaping...
            char next = s.peek();
            if (next == '{' || next == '}') {
                s.get();
                format_out.put(c);
                format_out.put(next);
                escaped = true;
                continue;
                // make sure to skip parsing but keep escape sequence.
            }
        } 
        if (c == '{') {
            parseFormat(args, s, format_out, parsed_args, has_reference_selection);
        } else {
            format_out.put(c);
        }
    }
    va_end(args);

    if (true == escaped || true == has_reference_selection) {
        format = format_out.str(); // this is only valid, because str() returns copy... otherwise, it is assumed both string variables point to the same underlying buffer? correct?
        format_out = std::basic_stringstream<char>();
        s = ByteEStream((uint8_t *)format.data(), format.size());
        while (true == s.good()) {
            c = s.get();
            if (c == '\\') {
                // escaping...
                char next = s.peek();
                if (next == '{' || next == '}') {
                    s.get();
                    format_out.put(next);
                    continue;
                    // make sure to skip parsing but undo escaping sequence.
                }
            } 
            if (c == '{') {
                parseReferenceFormat(parsed_args, s, format_out);
            } else {
                format_out.put(c);
            }
        }
    }
    deleteArgs(parsed_args);
    return format_out.str();
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