#include "json_parser.h"
#include "json_array.h"
#include "json_object.h"
#include "string_utils.h"

#include <stdbool.h>
#include <unistd.h>

#include <tuple>

#ifndef LOGGER_JSON_PARSER
#define LOGGER_JSON_PARSER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_PARSER
#include "logger.h"

#define ReaderTaskExact ReaderTaskExact<std::string>
#define ReaderTaskExtract ReaderTaskExtract<SharedArray<uint8_t>>

using namespace WylesLibs::Parser::Json;
using namespace WylesLibs::Parser;
using namespace WylesLibs::File;
using namespace WylesLibs;

static void readWhiteSpaceUntil(ByteEStream * r, std::string until);

// tree base
static void parseNumber(JsonArray * obj, ByteEStream * r);
static void parseString(JsonArray * obj, ByteEStream * r);
static void parseImmediate(JsonArray * obj, ByteEStream * r, std::string comp, JsonValue * value);

// base-ish (base and 1 at same time)...
static void parseNestedObject(JsonArray * obj, ByteEStream * r);
static void parseArray(JsonArray * obj, ByteEStream * r);

// 2 
static void parseValue(JsonArray * obj, ByteEStream * r);
static bool parseKey(JsonObject * obj, ByteEStream * r);

// 1
static void parseObject(JsonObject * obj, ByteEStream * r);

static void readWhiteSpaceUntil(ByteEStream * r, std::string until) {
    char c = r->peek();
    loggerPrintf(LOGGER_DEBUG, "Reading Whitespace Until: %s, %c\n", until.c_str(), c);
    if (until.find(c) != std::string::npos) {
        return;
    }
    while (STRING_UTILS_WHITESPACE.find(c) != std::string::npos) {
        r->get(); // consume whitespace...
        c = r->peek();
    }
    if (until.find(c) == std::string::npos) {
        std::string msg = "Only allow whitespace between tokens...";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    // cursor == until...
}

static void parseNumber(JsonArray * obj, ByteEStream * r) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Number\n");
    int8_t sign = 1;
    int8_t exponential_sign = 0;
    double exponential_multiplier = 1;
    double value = 0;

    size_t natural_digits = 1;
    size_t decimal_digits = 1;
    char c = r->peek();
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    if (c == '-') {
        sign = -1;
    }

    c = r->peek();
    if (isDigit(c)) {
        DecimalTuple t = r->read<DecimalTuple>();
        value = std::get<0>(t);
        natural_digits = std::get<1>(t);
        decimal_digits = std::get<2>(t);
    } else {
        // throw exception...
        std::string msg = "Invalid number.";
        loggerPrintf(LOGGER_INFO, "%s, found '%c'\n", msg.c_str(), c);
        throw std::runtime_error(msg);
    }

    std::string comp(" ,}\r\n\t");
    c = r->peek();
    if (c == 'e' || c == 'E') {
        c = r->peek();
        if (c == '-') { 
            exponential_sign = -1;
            r->get();
        } else if (c == '+') {
            exponential_sign = 1;
            r->get();
        }

        uint64_t exp = std::get<0>(r->read<NaturalTuple>());
        if (exp > FLT_MAX_EXP_ABS) {
            std::string msg = "parseNumber: exponential to large.";
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
        for (uint64_t x = 0; x < exp; x++) {
            exponential_multiplier *= 10;
        }
        loggerPrintf(LOGGER_DEBUG, "Exponential Sign: %d, Exponential Multiplier: %f\n", exponential_sign, exponential_multiplier);
    } else if (comp.find(c) == std::string::npos) { // if one of the characters is not in comp, throw exception...
        // if not the delimeter character then it's an invalid number.
        std::string msg = "Invalid number.";
        loggerPrintf(LOGGER_INFO, "%s, found '%c'\n", msg.c_str(), c);
        throw std::runtime_error(msg);
    }

    loggerPrintf(LOGGER_DEBUG, "Number before applying exponential: %f\n", value);

    if (exponential_sign == -1) {
        value = value / exponential_multiplier;
    } else if (exponential_sign == 1) {
        value = value * exponential_multiplier;
    }

    loggerPrintf(LOGGER_DEBUG, "Number after exponential: %f\n", value);

    obj->addValue((JsonValue *) new JsonNumber(value * sign, natural_digits, decimal_digits));

    loggerExec(LOGGER_DEBUG,
        if (true == r->good()) {
            loggerPrintf(LOGGER_DEBUG, "Parsed Number, stream is at '%c' [0x%02X]\n", r->peek(), r->peek());
        }
    );
}

static void parseString(JsonArray * obj, ByteEStream * r) {
    loggerPrintf(LOGGER_DEBUG, "Parsing String\n");

    r->get(); // consume starting quote

    char c = r->get();
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    std::string s;
    char prev_c = (char)0x00;
    size_t string_count = 0;
    while (MAX_LENGTH_OF_JSON_STRING_VALUES > string_count && (c != '"' || prev_c == '\\')) {
        /*
            '"'
            '\'
            '/'
            'b'
            'f'
            'n'
            'r'
            't'
            'u' hex hex hex hex    
        */
        if (prev_c == '\\') {
            if (c == 'b') { // backspace
                s += '\b';
            } else if (c == 'f') { // form feed
                s += '\f';
            } else if (c == 'n') { // newline
                s += '\n';
            } else if (c == 'r') { // carraige return
                s += '\r';
            } else if (c == 't') { // tab
                s += '\t';
            } else if (c == 'u') { // unicode
                for (uint8_t x = 0; x < 4; x = x + 2) {
                    // so, this takes a hex string and converts to it's binary value.
                    //  i.e. "0F" -> 0x0F;

                    // TODO: very lame that I have to cast to access public, overloaded functions from base class.
                    s += hexToChar(r->read<std::string>(2));
                }
            } else {
                // actual characters can just be appended.
                s += c;
            }
        } else {
            s += c;
        }
        string_count++;
        prev_c = c;
        c = r->get();
    }

    if (s.size() > MAX_LENGTH_OF_JSON_STRING_VALUES) {
        std::string msg = "parseString: string value too looooooonnnng!";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

    obj->addValue((JsonValue *) new JsonString(s));

    loggerExec(LOGGER_DEBUG,
        if (true == r->good()) {
            loggerPrintf(LOGGER_DEBUG, "Parsed String: %s, stream is at '%c' [0x%02X]\n", s.c_str(), r->peek(), r->peek());
        }
    );
}

static void parseImmediate(JsonArray * obj, ByteEStream * r, std::string comp, JsonValue * value) {
    loggerPrintf(LOGGER_DEBUG, "Parsing %s\n", comp.c_str());

    try {
        ReaderTaskExact task(comp, true); // lmao
        r->read<std::string>(comp.size(), &task);
        loggerPrintf(LOGGER_DEBUG, "Parsed %s, @ %c\n", comp.c_str(), r->peek());
        obj->addValue(value);
    } catch (std::exception& e) {
        std::string msg = "Invalid immediate value - throw away the whole object. Don't rest on your laurels!";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}

static void parseNestedObject(JsonArray * arr, ByteEStream * r) {
    char c = r->peek();
    if (c == '{') {
        JsonObject * new_obj = new JsonObject(arr->depth + 1);
        arr->addValue((JsonValue *)new_obj);
        parseObject((JsonObject *) new_obj, r);
    }
    loggerPrintf(LOGGER_DEBUG, "Returning object, found %c\n", c);
}

static void parseArray(JsonArray * arr, ByteEStream * r) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Array\n");

    char c = r->get();
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    while(c != ']') {
        if (c == '[' || c == ',') {
            parseValue(arr, r);
            loggerPrintf(LOGGER_DEBUG, "Returned from parseValue function.\n");
        }
        loggerExec(LOGGER_DEBUG,
            if (true == r->good()) {
                loggerPrintf(LOGGER_DEBUG, "%c\n", r->peek());
            }
        );
        c = r->get();
    }

    loggerExec(LOGGER_DEBUG,
        if (true == r->good()) {
            loggerPrintf(LOGGER_DEBUG, "Parsed Array, stream is at '%c', [%X]\n", r->peek(), r->peek());
        }
    );
}

static void parseValue(JsonArray * obj, ByteEStream * r) {
    char c = r->peek();
    bool parsed = false;
    // read until , or } or parsed token...
    while (c != ',' && c != '}' && c != ']') { 
        if (!parsed) {
            if (c == '"') {
                parseString(obj, r);
                parsed = true;
            } else if (isDigit(c) || c == '+' || c == '-') {
                parseNumber(obj, r);
                parsed = true;
            } else if (c == '[') {
                JsonArray * new_arr = new JsonArray(obj->depth + 1);
                obj->addValue(new_arr);
                parseArray(new_arr, r);
                parsed = true;
            } else if (c == 't') {
                std::string comp("true");
                JsonValue * boolean = (JsonValue *) new JsonBoolean(true);
                parseImmediate(obj, r, comp, boolean);
                parsed = true;
            } else if (c == 'f') {
                std::string comp("false");
                JsonValue * boolean = (JsonValue *) new JsonBoolean(false);
                parseImmediate(obj, r, comp, boolean);
                parsed = true;
            } else if (c == 'n') {
                std::string comp("null");
                JsonValue * nullValue = new JsonValue();
                parseImmediate(obj, r, comp, nullValue);
                parsed = true;
            } else if (c == '{') { 
                loggerPrintf(LOGGER_DEBUG, "Found object delimeter '%c'\n", c);
                parseNestedObject(obj, r);
                parsed = true;
            } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
                loggerPrintf(LOGGER_DEBUG, "Stream buffer dump: \n'%s'\n", r->stream_log.c_str());
#endif
                std::string msg = WylesLibs::format("Found non-whitespace char left of value token. '{c}'", c);
                loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            } else {
                // consume whitespace...
                r->get();
            }
        } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
            loggerPrintf(LOGGER_DEBUG, "Stream buffer dump: \n'%s'\n", r->stream_log.c_str());
#endif
            std::string msg = WylesLibs::format("Found non-whitespace char right of value token. '{c}'", c);
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        } else {
            // consume whitespace...
            r->get();
        }
        c = r->peek();
    }
    loggerExec(LOGGER_DEBUG,
        if (true == r->good()) {
            loggerPrintf(LOGGER_DEBUG, "Parsed Value, stream is at '%c', [0x%02X]\n", r->peek(), r->peek());
        }
    );
}

static bool parseKey(JsonObject * obj, ByteEStream * r) {
    loggerExec(LOGGER_DEBUG,
        if (true == r->good()) {
            loggerPrintf(LOGGER_DEBUG, "Parsing Key. %c\n", r->peek());
        }
    );

    ReaderTaskExtract extract('"', '"');
    SharedArray<uint8_t> key = r->read<SharedArray<uint8_t>>(":}", &extract);

    std::string key_string = key.toString();
    loggerPrintf(LOGGER_DEBUG, "Parsed Key String with delimeter: '%s'\n", key_string.c_str());
    size_t size = key_string.size();
    if (size < 1) {
        std::string msg = "Empty key string found.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    uint8_t until_match = key_string.substr(size-1, 1)[0];
    if (until_match == (uint8_t)'}') {
        loggerPrintf(LOGGER_DEBUG, "Found end of object. '%s'\n", key.toString().c_str());
        return false;
    }
    // uint8_t until_match = key.back();
    // if (until_match == (uint8_t)'}') {
    //     loggerPrintf(LOGGER_DEBUG, "Found end of object. '%s'\n", key.toString().c_str());
    //     return false;
    // }
    key_string = key_string.substr(0, size-1);
    if (size > MAX_LENGTH_OF_JSON_STRING_KEYS) {
        std::string msg = "Key string to loooooonnng!";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

    loggerExec(LOGGER_DEBUG,
        if (true == r->good()) {
            loggerPrintf(LOGGER_DEBUG, "Parsed Key String: '%s', stream is at '%c' [0x%02X]\n", key_string.c_str(), r->peek(), r->peek());
        }
    );

    obj->addKey(key_string);

    return true;
}

static void parseObject(JsonObject * obj, ByteEStream * r) {
    char c = r->get();
    while (c != '}') {
        if (c == '{' || c == ',') {
            if (!parseKey(obj, r)) {
                // end of obj.
                break;
            }
            parseValue(&(obj->values), r);
            c = r->get();
            loggerPrintf(LOGGER_DEBUG, "Returned from parseValue function. @ %c\n", c);
        }
    }

    loggerPrintf(LOGGER_DEBUG, "Broke out of key parsing loop...\n");
}

extern ESharedPtr<JsonValue> WylesLibs::Parser::Json::parseFile(ESharedPtr<StreamFactory> stream_factory, std::string file_path) {
    size_t i = 0;
    IStreamEStream r(stream_factory, file_path);
    return parse(dynamic_cast<ByteEStream *>(&r), i);
}

extern ESharedPtr<JsonValue> WylesLibs::Parser::Json::parse(std::string json) {
    loggerPrintf(LOGGER_DEBUG, "JSON: \n");
    loggerPrintf(LOGGER_DEBUG, "%s\n", json.c_str());
    if (json.size() > MAX_LENGTH_OF_JSON_STRING) {
        std::string msg = "String to loooonnnng!";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    size_t i = 0;
    ByteEStream r((uint8_t *)json.data(), json.size());
    return parse(&r, i);
}

extern ESharedPtr<JsonValue> WylesLibs::Parser::Json::parse(SharedArray<uint8_t> json) {
    if (json.size() > MAX_LENGTH_OF_JSON_STRING) {
        std::string msg = "Json data to loooonnnng!";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    size_t i = 0;

    ByteEStream r(json.begin(), json.size());
    return parse(&r, i);
}

// Let's define that parse function's start index is first index of token and end index is last index of token (one before delimeter).
//  Okay, I was convinced to use the string class for this lol... C++ book insists relatively low overhead when exceptions are thrown.
//      also, sizeof(pointers) < sizeof(std::string)
//      and .at() bounds checks (exceptions), [] doesn't
    // string construction? iterates over string to get length? that might be reason enough to change back to pointers lol... or maybe copy constructor is optimized? yeah
    //  but still that initial creation... 
extern ESharedPtr<JsonValue> WylesLibs::Parser::Json::parse(ByteEStream * r, size_t& i) {
    readWhiteSpaceUntil(r, "{[");

    ESharedPtr<JsonValue> obj;
    char c = r->peek();
    loggerPrintf(LOGGER_DEBUG, "First JSON character: %c\n", c);
    if (c == '{') {
        JsonObject * new_obj = new JsonObject(0);
        parseObject(new_obj, r);
        obj = ESharedPtr<JsonValue>(dynamic_cast<JsonValue *>(new_obj));
    } else if (c == '[') {
        JsonArray * new_arr = new JsonArray(0);
        // [1, 2, 3, 4] is valid JSON lol...
        parseArray(new_arr, r);
        obj = ESharedPtr<JsonValue>(dynamic_cast<JsonValue *>(new_arr));
    } else {
        std::string msg = "Invalid JSON data.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

    loggerPrintf(LOGGER_DEBUG, "Parsed JSON object. Returning to caller.\n");

    return obj;
}

extern std::string WylesLibs::Parser::Json::pretty(std::string json) {
    std::string pretty;
    uint8_t depth = 0;

    // LOL... allow spaces in between quotes!
    bool allow_spaces = false;
    for (auto c: json) {
        if (c == '{' || c == '[') {
            pretty += c;
            pretty += '\n';
            depth++;
            for (uint8_t i = 0; i < depth; i++) {
                pretty += '\t';
            }
        } else if (c == '}' || c == ']') {
            pretty += '\n';
            depth--;
            for (uint8_t i = 0; i < depth; i++) {
                pretty += '\t';
            }
            pretty += c;
        } else if (c == ',') {
            pretty += c;
            pretty += '\n';
            for (uint8_t i = 0; i < depth; i++) {
                pretty += '\t';
            }
        } else if (c == ':') {
            pretty += c;
            pretty += ' ';
        } else if (c == '\"') {
            allow_spaces = !allow_spaces;
            pretty += c;
        } else if (isAlpha(c) || isDigit(c) || c == '.' || (allow_spaces && c == ' ')) {
            pretty += c;
        }
    }

    return pretty;
}