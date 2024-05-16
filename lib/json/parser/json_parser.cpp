#include <stdbool.h>
#include "cstring.h"

#include "json_parser.h"
#include "json_array.h"
#include "json_object.h"

using namespace WylesLibs::Json;

static void readWhiteSpaceUntil(std::string buf, size_t& i, std::string until);
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
    if (until.contains(c)) {
        return;
    }
    while (whitespace.find(c) != std::string::npos) {
        c = buf.at(++i);
    }
    if (!until.contains(c)) {
        throw std::runtime_error("Only allow whitespace between tokens...");
    }
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
static void parseDecimal(std::string buf, size_t& i, double& value) {
    double decimal_divisor = 10;
    char c = buf.at(i);
    while (isDigit(c)) {
        value += (c - 0x30) / decimal_divisor;
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        decimal_divisor *= 10;
        c = buf.at(++i);
    }
    // make sure we point at last digit
    i--;
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseNatural(std::string buf, size_t& i, double& value) {
    char c = buf.at(i);
    while (isDigit(c)) {
        value = (value * 10) + (c - 0x30); 
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        c = buf.at(++i);
    }
    // make sure we point at last digit
    i--;
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseNumber(JsonArray * obj, std::string buf, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Number @ %lu\n", i);
    int8_t sign = 1;
    int8_t exponential_sign = 0;
    double exponential_multiplier = 10;
    double value = 0;

    // TODO: impose limit...

    char c = buf.at(i);
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    std::string comp(" ,}\r\n\t");
    while (comp.find(c) == std::string::npos) {
        if (isDigit(c)) {
            parseNatural(buf, i, value);
        } else if (c == '.') {
            parseDecimal(buf, ++i, value);
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
            parseNatural(buf, i, exp);
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
        obj->addValue(value);
    }
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseNestedObject(JsonArray * arr, std::string buf, size_t& i) {
    char c = buf.at(i);
    if (c == '{') {
        loggerPrintf(LOGGER_DEBUG, "Found %c @ %lu\n", c, i);
        // create new object... and update cursor/pointer to object.
        JsonObject * new_obj = new JsonObject();
        if (arr != nullptr) {
            arr->addValue((JsonValue *)new_obj);
        }
        loggerPrintf(LOGGER_DEBUG, "New OBJ, @ %lu\n", i);
        parseObject((JsonObject *) new_obj, buf, ++i);
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
        arr = new JsonArray(); 
        obj->addValue(arr);
    }

    char c = buf.at(i);
    loggerPrintf(LOGGER_DEBUG, "First char: %c\n", c);
    while(c != ']') {
        if (c == '[' || c == ',') { // only parseValue after start of new token... so shouldn't really matter where cursor as long as past previous.
            // TODO:
            //  nested arrays might be an issue. like too much recursion? curious what happens
            //  whitespace shouldn't though? lol
            //  nested object limit too..

            //  yeah might require some global state object... which means we can simplify parse value functions? and nested?
            parseValue(arr, buf, ++i);
            loggerPrintf(LOGGER_DEBUG, "Returned from parseArrayValue function.\n");
        }
        loggerPrintf(LOGGER_DEBUG, "%c\n", c);
    }

    loggerPrintf(LOGGER_DEBUG, "Parsed Array @ %lu\n", i);
}

// Let's define that parse function's start index is first index of token and end index is last index of token.
static void parseValue(JsonArray * obj, std::string buf, size_t& i) {
    char c = buf.at(i);
    bool parsed = false;
    // read until , or } or parsed token...
    while (c != ',' && c != '}') { 
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
                i += 3; // just consume null string... point to last character...
                // std::string comp("null");
                // JsonValue * nullValue = new JsonValue();
                // parseImmediate(obj, buf, i, &comp, nullValue);
      
                // hmm... this works but think about whether we want to just ignore malformed...
                //  when I think about validating input, ensureing full json, etc.
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
    // find start and end index of key string
    // TODO:
    //  if empty string...
    //  already contains,,
    //  maybe allow empty string but enforce uniqueness...
    char c = buf.at(i);
    while (c != '}') {
        if (c == '{' || c == ',') {
            loggerPrintf(LOGGER_DEBUG, "New Object Entry\n");
            parseKey(obj, buf, ++i);
            // i @ :
            parseValue(&(obj->values), buf, ++i);
            loggerPrintf(LOGGER_DEBUG, "Returned from parseValue function. @ %lu, %c\n", i, buf.at(i));

            // support {key:value,} - this is lame but a common thing? so let's support it.
            // peek to see if last element...
            size_t x = i;
            c = buf.at(x); // points to , or }
            if (c == ',') {
                while (whitespace.find(c) != std::string::npos) {
                    if (c == '}') {
                        i = x;
                        break;
                    }
                    c = buf.at(++x);
                } // found non whitespace character...
            }
        }
    }

    loggerPrintf(LOGGER_DEBUG, "Broke out of key parsing loop...\n");
}

// Let's define that parse function's start index is first index of token and end index is last index of token (one before delimeter).
//  Okay, I was convinced to use the string class for this lol... C++ book insists relatively low overhead when exceptions are thrown.
//      also, sizeof(pointers) < sizeof(std::string)
//      and .at() bounds checks (exceptions), [] doesn't
extern JsonValue * WylesLibs::Json::parse(std::string s, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "JSON: \n");
    loggerPrintf(LOGGER_DEBUG, "%s\n", s.c_str());

    readWhiteSpaceUntil(s, i, "{[");

    JsonValue * obj = nullptr;
    char c = s.at(i);
    loggerPrintf(LOGGER_DEBUG, "First JSON character: %c\n", c);
    if (c == '{') {
        JsonObject * new_obj = new JsonObject();
        parseObject(new_obj, s, i);
        obj = (JsonValue *) new_obj;
    } else if (c == '[') {
        JsonArray * new_obj = new JsonArray();
        // [1, 2, 3, 4] is valid JSON lol...
        parseArray(new_obj, s, i);
        obj = (JsonValue *) new_obj;
    }

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


// TODO: Once finished, think about how this can be implemented differently? ("take a step back", birds eye view, for better planning, better cache?)

//  what are we doing now... consume until start of object, 
//      then consume until end of key (:), then consume <value> and until (,) repeat until no more data...
//      so, not really strict json..., as in, some non-valid json will work...

//      for instance, don't need '}' and non-whitespace between tokens are valid... not among other things?

//  now, do I enforce whitespace?
//      to do this, parseObject and parseValue and mainloop need refactoring...
//      
//      require } before {?
//      this means, main loop needs refactoring...

// then I think that's it...
//  this is what the validation function was going to do... but let's just do it :)
