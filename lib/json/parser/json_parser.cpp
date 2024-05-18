#include <stdbool.h>
#include "cstring.h"

#include "json_parser.h"
#include "json_array.h"
#include "json_object.h"

#ifndef LOGGER_JSON_PARSER
#define LOGGER_JSON_PARSER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_PARSER
#include "logger.h"

using namespace WylesLibs::Json;

static void readWhiteSpaceUntil(std::string buf, size_t& i, std::string until);
static bool peekWhiteSpaceUntil(std::string buf, size_t& i, char until);

static size_t compareCString(std::string buf, std::string comp, size_t comp_length);

// tree base
static void parseDecimal(std::string buf, size_t& i, double& value);
static void parseNatural(std::string buf, size_t& i, double& value);
static void parseNumber(JsonArray * obj, std::string buf, size_t& i);
static void parseString(JsonArray * obj, std::string buf, size_t& i);
static void parseImmediate(JsonArray * obj, std::string buf, size_t& i, std::string comp, JsonValue * value);
static void parseKeyString(JsonObject * obj, std::string buf, size_t& i);

// base-ish (base and 1 at same time)...
static void parseNestedObject(JsonArray * obj, std::string buf, size_t& i);
static void parseArray(JsonArray * obj, std::string buf, size_t& i);

// 2 
static void parseValue(JsonArray * obj, std::string buf, size_t& i);
static void parseKey(JsonObject * obj, std::string buf, size_t& i);

// 1
static void parseObject(JsonObject * obj, std::string buf, size_t& i);

static std::string whitespace(" \r\n\t");
static void readWhiteSpaceUntil(std::string buf, size_t& i, std::string until) {
    char c = buf.at(i);
    loggerPrintf(LOGGER_DEBUG, "Reading Whitespace Until: %s, @ %lu, %c\n", until.c_str(), i, c);
    if (until.find(c) != std::string::npos) {
        return;
    }
    while (whitespace.find(c) != std::string::npos) {
        c = buf.at(++i);
    }
    if (until.find(c) == std::string::npos) {
        throw std::runtime_error("Only allow whitespace between tokens...");
    }
}

static bool peekWhiteSpaceUntil(std::string buf, size_t& i, char until) {
    size_t x = i;
    char c = buf.at(++x);
    if (c == until) {
        i = x;
        return true;
    }
    while (whitespace.find(c) != std::string::npos) {
        c = buf.at(++x);
    } // found non whitespace character...
    if (c == until) {
        i = x;
        return true;
    }

    return false;
}

static size_t compareCString(std::string buf, std::string comp, size_t comp_length) {
    size_t x = 0;
    while(x < comp_length) {
        if (comp.at(x) != buf.at(x)) {
            break;
        }
        x++;
    }
    return x;
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseDecimal(std::string buf, size_t& i, double& value, size_t& digit_count) {
    double decimal_divisor = 10;
    char c = buf.at(i);
    while (isDigit(c)) {
        value += (c - 0x30) / decimal_divisor;
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        decimal_divisor *= 10;
        c = buf.at(++i);
        if (++digit_count > FLT_MAX_MIN_DIGITS) {
            throw std::runtime_error("parseDecimal: Exceeded decimal digit limit.");
        }
    }
    // make sure we point at last digit
    i--;
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseNatural(std::string buf, size_t& i, double& value, size_t& digit_count) {
    char c = buf.at(i);
    while (isDigit(c)) {
        value = (value * 10) + (c - 0x30); 
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        c = buf.at(++i);
        if (++digit_count > FLT_MAX_MIN_DIGITS) {
            throw std::runtime_error("parseNatural: Exceeded natural digit limit.");
        }
    }
    // make sure we point at last digit
    i--;
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseNumber(JsonArray * obj, std::string buf, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Number @ %lu\n", i);
    int8_t sign = 1;
    int8_t exponential_sign = 0;
    double exponential_multiplier = 1;
    double value = 0;

    size_t natural_digits = 1;
    size_t decimal_digits = 1;
    char c = buf.at(i);
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    std::string comp(" ,}\r\n\t");
    while (comp.find(c) == std::string::npos) {
        if (isDigit(c)) {
            parseNatural(buf, i, value, natural_digits);
        } else if (c == '.') {
            parseDecimal(buf, ++i, value, decimal_digits);
        } else if (c == 'e' || c == 'E') {
            c = buf.at(++i);
            if (c == '-') { 
                exponential_sign = -1;
                i++;
            } else if (c == '+') {
                exponential_sign = 1;
                i++;
            }

            double exp = 0;
            size_t dummy_digit_count = 0;
            parseNatural(buf, i, exp, dummy_digit_count);
            
            if (exp > FLT_MAX_EXP_ABS) {
                throw std::runtime_error("parseNumber: exponential to large.");
            }
            for (size_t x = 0; x < exp; x++) {
                exponential_multiplier *= 10;
            }
            loggerPrintf(LOGGER_DEBUG, "Exponential Sign: %d, Exponential Multiplier: %f\n", exponential_sign, exponential_multiplier);
        } else if (buf.at(i) == '-') {
            sign = -1;
        } 
        // Consume invalid data in-between tokens...
        // else if (buf.at(i) == '+') {} 
        // else {} 
        c = buf.at(++i);
    }
    // make sure we point at last digit
    i--;

    loggerPrintf(LOGGER_DEBUG, "Number before applying exponential: %f\n", value);

    if (exponential_sign == -1) {
        value = value / exponential_multiplier;
    } else if (exponential_sign == 1) {
        value = value * exponential_multiplier;
    }

    loggerPrintf(LOGGER_DEBUG, "Number after exponential: %f\n", value);

    obj->addValue((JsonValue *) new JsonNumber(value * sign));

    loggerPrintf(LOGGER_DEBUG, "Parsed Number @ %lu\n", i);
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseString(JsonArray * obj, std::string buf, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "Parsing String @ %lu\n", i);

    char c = buf.at(++i);
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
                    // s += hexToChar(buf.at(i + x));
                }
                // because indexing starts at 0 and x < 4 lol...
                i += 3;
            } else {
                // actual characters can just be appended.
                s += c;
            }
        } else {
            s += c;
        }
        prev_c = c;
        c = buf.at(++i);
    }

    if (s.size() > MAX_LENGTH_OF_JSON_STRING_VALUES) {
        throw std::runtime_error("parseString: string value too looooooonnnng!");
    }

    obj->addValue((JsonValue *) new JsonString(s));

    loggerPrintf(LOGGER_DEBUG, "Parsed String: %s\n", s.c_str());
    loggerPrintf(LOGGER_DEBUG, "Parsed String @ %lu\n", i);
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseImmediate(JsonArray * obj, std::string buf, size_t& i, std::string comp, JsonValue * value) {
    loggerPrintf(LOGGER_DEBUG, "Parsing %s @ %lu\n", comp.c_str(), i);

    std::string buf_i = buf.substr(i, comp.size());
    size_t consumed = compareCString(buf_i, comp, comp.size());
    i += consumed - 1; // point at last index of token

    if (consumed == comp.size()) {
        loggerPrintf(LOGGER_DEBUG, "Parsed %s @ %lu, %c\n", comp.c_str(), i, buf.at(i));

        // kind of annoying but we want to parse null and not include it in intermediate structure...
        if (value->type != NULL_TYPE) {
            obj->addValue(value);
        }
    } else {
        throw std::runtime_error("parseImmediate: invalid immediate value. Throw away the whole object lol.");
    }
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseNestedObject(JsonArray * arr, std::string buf, size_t& i) {
    char c = buf.at(i);
    if (c == '{') {
        loggerPrintf(LOGGER_DEBUG, "Found %c @ %lu\n", c, i);
        // create new object... and update cursor/pointer to object.
        JsonObject * new_obj = new JsonObject(arr->depth + 1);
        // TODO: this 
        if (arr != nullptr) {
            // this should never be null.. but lets
            arr->addValue((JsonValue *)new_obj);
        } else {
            // TODO: better exception messages...
            throw std::runtime_error("parseNestedObject: parent array null");
        }
        loggerPrintf(LOGGER_DEBUG, "New OBJ, @ %lu\n", i);
        parseObject((JsonObject *) new_obj, buf, i);
    }
    loggerPrintf(LOGGER_DEBUG, "Returning object, found %c @ %lu\n", c, i);
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseArray(JsonArray * obj, std::string buf, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Array @ %lu\n", i);
    
    JsonArray * arr;
    if (i == 0) {
        arr = obj;
    } else {
        arr = new JsonArray(obj->depth + 1); 
        obj->addValue(arr);
    }

    char c = buf.at(i);
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    while(c != ']') {
        if (c == '[' || c == ',') { 
            // peek for at end of object...
            if (peekWhiteSpaceUntil(buf, i, ']')) break;

            parseValue(arr, buf, ++i);
            loggerPrintf(LOGGER_DEBUG, "Returned from parseValue function.\n");
        }
        loggerPrintf(LOGGER_DEBUG, "%c\n", buf.at(i));
        c = buf.at(i);
    }

    loggerPrintf(LOGGER_DEBUG, "Parsed Array @ %lu\n", i);
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseValue(JsonArray * obj, std::string buf, size_t& i) {
    char c = buf.at(i);
    bool parsed = false;
    // read until , or } or parsed token...
    while (c != ',' && c != '}' && c != ']') { 
        if (!parsed) {
            if (c == '"') {
                parseString(obj, buf, i);
                parsed = true;
            } else if (isDigit(c) || c == '+' || c == '-') {
                parseNumber(obj, buf, i);
                parsed = true;
            } else if (c == '[') {
                parseArray(obj, buf, i);
                parsed = true;
            } else if (c == 't') {
                std::string comp("true");
                JsonValue * boolean = (JsonValue *) new JsonBoolean(true);
                parseImmediate(obj, buf, i, comp, boolean);
                parsed = true;
            } else if (c == 'f') {
                std::string comp("false");
                JsonValue * boolean = (JsonValue *) new JsonBoolean(false);
                parseImmediate(obj, buf, i, comp, boolean);
                parsed = true;
            } else if (c == 'n') {
                std::string comp("null");
                JsonValue * nullValue = new JsonValue();
                parseImmediate(obj, buf, i, comp, nullValue);
                parsed = true;
            } else if (c == '{') { 
                loggerPrintf(LOGGER_DEBUG, "Found object delimeter '%c' @ %lu\n", c, i);
                parseNestedObject(obj, buf, i);
                parsed = true;
            } else if (whitespace.find(c) == std::string::npos) {
                throw std::runtime_error("Non-whitespace found in parseValue");
            }
        } else if (whitespace.find(c) == std::string::npos) {
            throw std::runtime_error("Non-whitespace found in parseValue");
        }
        c = buf.at(++i);
    }
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseKeyString(JsonObject * obj, std::string buf, size_t& i) {
    size_t start_i = i;
    char c = buf.at(i);
    while (c != '"') {
        c = buf.at(++i);
    } // found trailing quote...
    size_t size = i - start_i;
    if (size == 0) {
        throw std::runtime_error("Empty key string found.");
    } else if (size > MAX_LENGTH_OF_JSON_STRING_KEYS) {
        throw std::runtime_error("parseKeyString: Key string to loooooonnng!");
    }
    std::string s = buf.substr(start_i, size);
    obj->addKey(s);
    loggerPrintf(LOGGER_DEBUG, "Parsed Key String: %s @ %lu\n", s.c_str(), i);
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseKey(JsonObject * obj, std::string buf, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Key. @ %lu, %c\n", i, buf.at(i));
    readWhiteSpaceUntil(buf, i, "\"");
    // i @ "
    parseKeyString(obj, buf, ++i);
    // i @ "
    readWhiteSpaceUntil(buf, ++i, ":");
    // i @ :
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseObject(JsonObject * obj, std::string buf, size_t& i) {
    char c = buf.at(i);
    while (c != '}') {
        if (c == '{' || c == ',') {
            loggerPrintf(LOGGER_DEBUG, "New Object Entry\n");

            // peek for at end of object...
            if (peekWhiteSpaceUntil(buf, i, '}')) break;

            parseKey(obj, buf, ++i);
            // i @ :
            parseValue(&(obj->values), buf, ++i);
            c = buf.at(i);
            loggerPrintf(LOGGER_DEBUG, "Returned from parseValue function. @ %lu, %c\n", i, c);
        }
    }

    loggerPrintf(LOGGER_DEBUG, "Broke out of key parsing loop...\n");
}

// Let's define that parse function's start index is first index of token and end index is last index of token (one before delimeter).
//  Okay, I was convinced to use the string class for this lol... C++ book insists relatively low overhead when exceptions are thrown.
//      also, sizeof(pointers) < sizeof(std::string)
//      and .at() bounds checks (exceptions), [] doesn't
    // string construction? iterates over string to get length? that might be reason enough to change back to pointers lol... or maybe copy constructor is optimized? yeah
    //  but still that initial creation... 
extern JsonValue * WylesLibs::Json::parse(std::string s, size_t& i) {
    if (s.size() > MAX_LENGTH_OF_JSON_STRING) {
        throw std::runtime_error("WylesLibs::Json::parse: String to loooonnnng!");
    }
    loggerPrintf(LOGGER_DEBUG, "JSON: \n");
    loggerPrintf(LOGGER_DEBUG, "%s\n", s.c_str());

    readWhiteSpaceUntil(s, i, "{[");

    JsonValue * obj = nullptr;
    char c = s.at(i);
    loggerPrintf(LOGGER_DEBUG, "First JSON character: %c\n", c);
    if (c == '{') {
        JsonObject * new_obj = new JsonObject(0);
        parseObject(new_obj, s, i);
        obj = (JsonValue *) new_obj;
    } else if (c == '[') {
        JsonArray * new_obj = new JsonArray(0);
        // [1, 2, 3, 4] is valid JSON lol...
        parseArray(new_obj, s, i);
        obj = (JsonValue *) new_obj;
    }

    loggerPrintf(LOGGER_DEBUG, "Parsed JSON object. Returning to caller.\n");

    return obj;
}

extern std::string WylesLibs::Json::pretty(std::string json) {
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