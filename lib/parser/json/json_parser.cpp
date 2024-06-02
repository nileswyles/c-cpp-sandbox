#include <stdbool.h>
#include <fcntl.h>
#include "string_utils.h"

#include "json_parser.h"
#include "json_array.h"
#include "json_object.h"

// #include "message_formatter.h"

#ifndef LOGGER_JSON_PARSER
#define LOGGER_JSON_PARSER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_PARSER
#include "logger.h"

using namespace WylesLibs::Parser::Json;
using namespace WylesLibs;

static void readWhiteSpaceUntil(Reader * r, std::string until);
static bool peekWhiteSpaceUntil(Reader * r, char until);

static size_t compareCString(Reader * r, std::string comp, size_t comp_length);

// tree base
static void parseDecimal(Reader * r, double& value);
static void parseNatural(Reader * r, double& value);
static void parseNumber(JsonArray * obj, Reader * r);
static void parseString(JsonArray * obj, Reader * r);
static void parseImmediate(JsonArray * obj, Reader * r, std::string comp, JsonValue * value);

// base-ish (base and 1 at same time)...
static void parseNestedObject(JsonArray * obj, Reader * r);
static void parseArray(JsonArray * obj, Reader * r);

// 2 
static void parseValue(JsonArray * obj, Reader * r);
static bool parseKey(JsonObject * obj, Reader * r);

// 1
static void parseObject(JsonObject * obj, Reader * r);

static void readWhiteSpaceUntil(Reader * r, std::string until) {
    char c = r->peekByte();
    loggerPrintf(LOGGER_DEBUG, "Reading Whitespace Until: %s, %c\n", until.c_str(), c);
    if (until.find(c) != std::string::npos) {
        return;
    }
    while (STRING_UTILS_WHITESPACE.find(c) != std::string::npos) {
        r->readByte(); // consume whitespace...
        c = r->peekByte();
    }
    if (until.find(c) == std::string::npos) {
        std::string msg = "Only allow whitespace between tokens...";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    // cursor == until...
}

static size_t compareCString(std::string actual, std::string comp, size_t comp_length) {
    size_t x = 0;
    while(x < comp_length) {
        if (comp.at(x) != actual.at(x)) {
            break;
        }
        x++;
    }
    return x;
}

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

static void parseNumber(JsonArray * obj, Reader * r) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Number\n");
    int8_t sign = 1;
    int8_t exponential_sign = 0;
    double exponential_multiplier = 1;
    double value = 0;

    size_t natural_digits = 1;
    size_t decimal_digits = 1;
    char c = r->peekByte();
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    std::string comp(" ,}\r\n\t");
    while (comp.find(c) == std::string::npos) {
        if (isDigit(c)) {
            parseNatural(r, value, natural_digits);
        } else if (c == '.') {
            r->readByte();
            parseDecimal(r, value, decimal_digits);
        } else if (c == 'e' || c == 'E') {
            c = r->peekByte();
            if (c == '-') { 
                exponential_sign = -1;
                r->readByte();
            } else if (c == '+') {
                exponential_sign = 1;
                r->readByte();
            }

            double exp = 0;
            size_t dummy_digit_count = 0;
            parseNatural(r, exp, dummy_digit_count);
            
            if (exp > FLT_MAX_EXP_ABS) {
                std::string msg = "parseNumber: exponential to large.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            for (size_t x = 0; x < exp; x++) {
                exponential_multiplier *= 10;
            }
            loggerPrintf(LOGGER_DEBUG, "Exponential Sign: %d, Exponential Multiplier: %f\n", exponential_sign, exponential_multiplier);
        } else if (c == '-') {
            sign = -1;
        } 
        // Consume invalid data in-between tokens...
        // else if (c == '+') {} 
        // else {} 
        c = r->peekByte();
    }

    loggerPrintf(LOGGER_DEBUG, "Number before applying exponential: %f\n", value);

    if (exponential_sign == -1) {
        value = value / exponential_multiplier;
    } else if (exponential_sign == 1) {
        value = value * exponential_multiplier;
    }

    loggerPrintf(LOGGER_DEBUG, "Number after exponential: %f\n", value);

    obj->addValue((JsonValue *) new JsonNumber(value * sign));

    loggerPrintf(LOGGER_DEBUG, "Parsed Number\n");
}

static void parseString(JsonArray * obj, Reader * r) {
    loggerPrintf(LOGGER_DEBUG, "Parsing String\n");

    r->readByte(); // consume starting quote

    char c = r->readByte();
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    std::string s;
    char prev_c = (char)0x00;
    // TODO: set limit... of strings...
    while (c != '"') {
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
                // TODO:
                // this can be better
                for (size_t x = 0; x < 4; x = x + 2) {
                    // so, this takes a hex string and converts to it's binary value.
                    //  i.e. "0F" -> 0x0F;
                    // s += hexToChar(r.at(i + x));
                }
                // because indexing starts at 0 and x < 4 lol...
                r->readBytes(3);
            } else {
                // actual characters can just be appended.
                s += c;
            }
        } else {
            s += c;
        }
        prev_c = c;
        c = r->readByte();
    }

    if (s.size() > MAX_LENGTH_OF_JSON_STRING_VALUES) {
        std::string msg = "parseString: string value too looooooonnnng!";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

    obj->addValue((JsonValue *) new JsonString(s));

    loggerPrintf(LOGGER_DEBUG, "Parsed String: %s\n", s.c_str());
}

static void parseImmediate(JsonArray * obj, Reader * r, std::string comp, JsonValue * value) {
    loggerPrintf(LOGGER_DEBUG, "Parsing %s\n", comp.c_str());

    std::string actual = r->readBytes(comp.size()).toString();
    size_t consumed = compareCString(actual, comp, comp.size());
    if (consumed == comp.size()) {
        loggerPrintf(LOGGER_DEBUG, "Parsed %s, @ %c\n", comp.c_str(), r->peekByte());

        // kind of annoying but we want to parse null and not include it in intermediate structure...
        if (value->type != NULL_TYPE) {
            obj->addValue(value);
        }
    } else {
        std::string msg = "Invalid immediate value. Throw away the whole object lol.";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}

static void parseNestedObject(JsonArray * arr, Reader * r) {
    char c = r->peekByte();
    if (c == '{') {
        loggerPrintf(LOGGER_DEBUG, "Found %c\n", c);
        // create new object... and update cursor/pointer to object.
        JsonObject * new_obj = new JsonObject(arr->depth + 1);
        // TODO: this 
        if (arr != nullptr) {
            // this should never be null.. but lets
            arr->addValue((JsonValue *)new_obj);
        } else {
            std::string msg = "Parent array null";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
        loggerPrintf(LOGGER_DEBUG, "New OBJ\n");
        parseObject((JsonObject *) new_obj, r);
    }
    loggerPrintf(LOGGER_DEBUG, "Returning object, found %c\n", c);
}

static void parseArray(JsonArray * obj, Reader * r) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Array\n");
    
    JsonArray * arr;
    if (obj->depth == 0) {
        arr = obj;
    } else {
        arr = new JsonArray(obj->depth + 1); 
        obj->addValue(arr);
    }

    char c = r->readByte();
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    while(c != ']') {
        if (c == '[' || c == ',') { 
            parseValue(arr, r);
            loggerPrintf(LOGGER_DEBUG, "Returned from parseValue function.\n");
        }
        loggerPrintf(LOGGER_DEBUG, "%c\n", r->peekByte());
        c = r->readByte();
    }

    loggerPrintf(LOGGER_DEBUG, "Parsed Array\n");
}

static void parseValue(JsonArray * obj, Reader * r) {
    char c = r->peekByte();
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
                parseArray(obj, r);
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
                std::string msg = "Non-whitespace found left of value token.";
                loggerPrintf(LOGGER_ERROR, "%s '%c'\n", msg.c_str(), c);
                throw std::runtime_error(msg);
            } else {
                // consume whitespace...
                r->readByte();
            }
        } else if (STRING_UTILS_WHITESPACE.find(c) == std::string::npos) {
            std::string msg = "Non-whitespace found right of value token.";
            loggerPrintf(LOGGER_ERROR, "%s '%c'\n", msg.c_str(), c);
            throw std::runtime_error(msg);
        } else {
            // consume whitespace...
            r->readByte();
        }
        c = r->peekByte();
    }
}

static bool parseKey(JsonObject * obj, Reader * r) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Key. %c\n", r->peekByte());

    ReaderTaskExtract extract('"', '"');
    Array<uint8_t> key = r->readUntil(":}", &extract);

    // This is actually the more correct way to check this... why you no work?
    // printf("BACK: %c\n", (char)key.back());
    // if ((char)key.back() == '}') {
    //     return false;
    // }
    std::string key_string = key.toString();
    size_t size = key_string.size();
    if (size == 0) {
        std::string msg = "Empty key string found.";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    } else if (size > MAX_LENGTH_OF_JSON_STRING_KEYS) {
        std::string msg = "Key string to loooooonnng!";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    } else if (key_string == "}") {
        return false;
    }

    loggerPrintf(LOGGER_DEBUG, "Parsed Key String: %s\n", key_string.c_str());

    return true;
}

static void parseObject(JsonObject * obj, Reader * r) {
    char c = r->readByte();
    while (c != '}') {
        if (c == '{' || c == ',') {
            loggerPrintf(LOGGER_DEBUG, "New Object Entry\n");
            if (!parseKey(obj, r)) {
                // end of obj.
                break;
            }
            parseValue(&(obj->values), r);
            c = r->readByte();
            loggerPrintf(LOGGER_DEBUG, "Returned from parseValue function. @ %c\n", c);
        }
    }

    loggerPrintf(LOGGER_DEBUG, "Broke out of key parsing loop...\n");
}

extern JsonValue * WylesLibs::Parser::Json::parseFile(std::string file_path) {
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1) {
        std::string msg = "File does not exist.";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    size_t i = 0;
    Reader r(fd);
    return parse(&r, i);
}

extern JsonValue * WylesLibs::Parser::Json::parse(std::string json) {
    loggerPrintf(LOGGER_DEBUG, "JSON: \n");
    loggerPrintf(LOGGER_DEBUG, "%s\n", json.c_str());
    if (json.size() > MAX_LENGTH_OF_JSON_STRING) {
        std::string msg = "String to loooonnnng!";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    size_t i = 0;
    Reader r((uint8_t *)json.data(), json.size());
    return parse(&r, i);
}

extern JsonValue * WylesLibs::Parser::Json::parse(Array<uint8_t> json) {
    if (json.size() > MAX_LENGTH_OF_JSON_STRING) {
        std::string msg = "Json data to loooonnnng!";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    size_t i = 0;
    Reader r(json.buf, json.size());
    return parse(&r, i);
}

// Let's define that parse function's start index is first index of token and end index is last index of token (one before delimeter).
//  Okay, I was convinced to use the string class for this lol... C++ book insists relatively low overhead when exceptions are thrown.
//      also, sizeof(pointers) < sizeof(std::string)
//      and .at() bounds checks (exceptions), [] doesn't
    // string construction? iterates over string to get length? that might be reason enough to change back to pointers lol... or maybe copy constructor is optimized? yeah
    //  but still that initial creation... 
extern JsonValue * WylesLibs::Parser::Json::parse(Reader * r, size_t& i) {
    readWhiteSpaceUntil(r, "{[");

    JsonValue * obj = nullptr;
    char c = r->peekByte();
    loggerPrintf(LOGGER_DEBUG, "First JSON character: %c\n", c);
    if (c == '{') {
        JsonObject * new_obj = new JsonObject(0);
        parseObject(new_obj, r);
        obj = (JsonValue *) new_obj;
    } else if (c == '[') {
        JsonArray * new_obj = new JsonArray(0);
        // [1, 2, 3, 4] is valid JSON lol...
        parseArray(new_obj, r);
        obj = (JsonValue *) new_obj;
    } else {
        std::string msg = "Invalid JSON data.";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

    loggerPrintf(LOGGER_DEBUG, "Parsed JSON object. Returning to caller.\n");

    return obj;
}

extern std::string WylesLibs::Parser::Json::pretty(std::string json) {
    std::string pretty;
    size_t depth = 0;

    // LOL... allow spaces in between quotes!
    bool allow_spaces = false;
    for (auto c: json) {
        if (c == '{' || c == '[') {
            pretty += c;
            pretty += '\n';
            depth++;
            for (size_t i = 0; i < depth; i++) {
                pretty += '\t';
            }
        } else if (c == '}' || c == ']') {
            pretty += '\n';
            depth--;
            for (size_t i = 0; i < depth; i++) {
                pretty += '\t';
            }
            pretty += c;
        } else if (c == ',') {
            pretty += c;
            pretty += '\n';
            for (size_t i = 0; i < depth; i++) {
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