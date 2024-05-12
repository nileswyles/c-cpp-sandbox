#include <stdbool.h>
#include "json.h"

using namespace WylesLibs::Json;

static size_t compareCString(std::string * buf, std::string * comp, size_t comp_length);

static void parseDecimal(std::string * buf, size_t& i, double& value);
static void parseNatural(std::string * buf, size_t& i, double& value);
static void parseNumber(JsonObject * obj, std::string * buf, size_t& i);
static void parseString(JsonObject * obj, std::string * buf, size_t& i);
static void parseArray(JsonObject * obj, std::string * buf, size_t& i);
static void parseImmediate(JsonObject * obj, std::string * buf, size_t& i, std::string * comp, JsonValue * value);
static void parseValue(JsonObject * obj, std::string * buf, size_t& i, const char stop);
static void parseKey(JsonObject * obj, std::string * buf, size_t& i);

static size_t compareCString(std::string * buf, std::string * comp, size_t comp_length) {
    size_t x = 0;
    while(x < comp_length) {
        if (comp->at(x) != buf->at(x)) {
            break;
        }
        x++;
    }
    return x;
}

static void parseDecimal(std::string * buf, size_t& i, double& value) {
    double decimal_divisor = 10;
    char c = buf->at(i);
    while (isDigit(c)) { // iterate non-digit. likely until comma, whitespace or exponential
        // 1.1234567
        // 1 + .1
        // 1.1 + .02
        value += (c - 0x30) / decimal_divisor;
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        decimal_divisor *= 10;
        c = buf->at(++i);
    }
    // make sure we point at last digit
    i--;
}

static void parseNatural(std::string * buf, size_t& i, double& value) {
    char c = buf->at(i);
    while (isDigit(c)) { // iterate non-digit. likely until comma, whitespace or exponential
        // 1
        // 1 * 10 = 10 + 2;
        // 12 * 10 = 120 + 3; 
        value = (value * 10) + (c - 0x30); 
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        c = buf->at(++i);
    }
    // make sure we point at last digit
    i--;
}

static void parseNumber(JsonObject * obj, std::string * buf, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Number @ %lu\n", i);

    int8_t sign = 1;

    int8_t exponential_sign = 0;
    double exponential_multiplier = 10;

    double value = 0;
    char c = buf->at(i);

    std::string comp(" ,}\r\n\t");
    while (comp.find(c) != std::string::npos) {
        if (isDigit(c)) {
            parseNatural(buf, i, value);
        } else if (c == '.') {
            parseDecimal(buf, ++i, value);
        } else if (c == 'e' || c == 'E') {
            c = buf->at(++i);
            if (c == '-') { 
                exponential_sign = -1;
                i++;
            } else if (c == '+') {
                exponential_sign = 1;
                i++;
            }

            double exp = 0;
            parseNatural(buf, i, exp);

            // 1 = * 10
            // 2 = * 100
            // 3 = * 1000
            for (size_t x = 0; x < exp; x++) {
                exponential_multiplier *= 10;
            }
            loggerPrintf(LOGGER_DEBUG, "Exponential Sign: %d, Exponential Multiplier: %f\n", exponential_sign, exponential_multiplier);
        } else if (buf->at(i) == '-') {
            sign = -1;
        } else if (buf->at(i) == '+') {
        } else { // parse number value...
            // throw exception/break, do something...?, move to while condition?
        }
        c = buf->at(++i);
    }

    loggerPrintf(LOGGER_DEBUG, "Number before applying exponential: %f\n", value);

    if (exponential_sign == -1) {
        value = value / exponential_multiplier;
    } else if (exponential_sign == 1) {
        value = value * exponential_multiplier;
    }

    loggerPrintf(LOGGER_DEBUG, "Number after exponential: %f\n", value);

    JsonValue * num = (JsonValue *) new JsonNumber(value * sign);
    (obj->values).append(num);

    loggerPrintf(LOGGER_DEBUG, "Parsed Number @ %lu\n", i);
}

static void parseString(JsonObject * obj, std::string * buf, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "Parsing String @ %lu\n", i);

    char c = buf->at(i);
    std::string s;
    char prev_c = (char)0x00;
    while (c != '"') {
        // start of escape sequence...

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
            // \ gets thrown out but let's process some unicode...
            // convert and append...

            // lib doesn't handle this right? only for literals? lmao
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
                    s += hexToChar(buf + i + x);
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
        c = buf->at(++i);
    }

    // lol... not the best but
    //  who cares rn ... value value value value
    JsonValue * jsonString = (JsonValue *) new JsonString(s);
    (obj->values).append(jsonString);

    loggerPrintf(LOGGER_DEBUG, "Parsed String: %s\n", s.c_str());
    loggerPrintf(LOGGER_DEBUG, "Parsed String @ %lu\n", i);
}

static void parseArray(JsonObject * obj, std::string * buf, size_t& i) {
    loggerPrintf(LOGGER_DEBUG, "Parsing Array @ %lu\n", i);

    char c = buf->at(i);
    while(c != ']') {
        // TODO: check if malformed array... and do something... bubble it up!
        // can either copy string and null terminate or pass delimeter to parseValue...
        parseValue(obj, buf, i, ',');
    }

    loggerPrintf(LOGGER_DEBUG, "Parsed Array @ %lu\n", i);
}

static void parseImmediate(JsonObject * obj, std::string * buf, size_t& i, std::string * comp, JsonValue * value) {
    loggerPrintf(LOGGER_DEBUG, "Parsing %s @ %lu\n", comp->c_str(), i);

    std::string buf_i = buf->substr(i, comp->size());
    size_t consumed = compareCString(&buf_i, comp, comp->size());
    if (consumed == comp->size()) {
        (obj->values).append(value);
    }
    i += consumed - 1; // point at last index of token

    loggerPrintf(LOGGER_DEBUG, "Parsed %s @ %lu\n", comp->c_str(), i);
}

static void parseValue(JsonObject * obj, std::string * buf, size_t& i, const char stop) {
    char c = buf->at(i);
    while (c != stop) {
        // loggerPrintf(LOGGER_DEBUG, "index: %lu\n", i);
        // loggerPrintf(LOGGER_DEBUG, "Found %c @ %lu\n", c, i);
        // TODO:
        //  if's vs switches...?
        if (c == '"') {
            parseString(obj, buf, ++i);
            break;
        } else if (isDigit(c) || c == '+' || c == '-') {
            parseNumber(obj, buf, i);
            break;
        } else if (c == '[') {
            parseArray(obj, buf, ++i);
            break;
        } else if (c == 't') {
            std::string comp("true");
            JsonValue * boolean = (JsonValue *) new JsonBoolean(true);
            parseImmediate(obj, buf, i, &comp, boolean);
            break;
        } else if (c == 'f') {
            std::string comp("false");
            JsonValue * boolean = (JsonValue *) new JsonBoolean(false);
            parseImmediate(obj, buf, i, &comp, boolean);
            break;
        } else if (c == 'n') {
            std::string comp("null");
            JsonValue * nullValue = new JsonValue();
            parseImmediate(obj, buf, i, &comp, nullValue);
            break;
            // TODO:
            // hmm... this works but think about whether we want to just ignore malformed...
            //  when I think about validating input, ensureing full json, etc.
        } else if (c == '}') {
            break; // don't increment pointer and break;
        }
        c = buf->at(++i);
    }
}

static void parseKey(JsonObject * obj, std::string * buf, size_t& i) {
    // find start and end index of key string
    // TODO:
    //  if empty string...
    //  already contains,,

    //  maybe allow empty string but enforce uniqueness...

    // might be worth implementing a map structure eventually.
    char c = buf->at(i);
    while (c != '"') {
        c = buf->at(++i);
    } // found quote
    size_t start_i = ++i;

    c = buf->at(i);
    while (c != '"') {
        c = buf->at(++i);
    } // found quote
    size_t size = i - start_i;
    if (size == 0) {
        // TODO:
    }

    std::string s = buf->substr(start_i, size);
    loggerPrintf(LOGGER_DEBUG, "Parsed Key String: %s @ %lu\n", s.c_str(), i);
    if ((obj->keys).uniqueAppend(s) != OPERATION_SUCCESS) {
        // TODO:
    }

    // value needs to be preceded by key
    while (c != ':') {
        c = buf->at(++i);
    }
    loggerPrintf(LOGGER_DEBUG, "Found %c @ %lu\n", c, i);
    parseValue(obj, buf, ++i, (char)0x00);
}

// Let's define that parse functions start index is first index of token and end index is last index of token.
//  Okay, I'm convinced to switch to string class for this lol... C++ book insists relatively low overhead when exceptions are thrown.
//      also, sizeof(pointers) < sizeof(std::string)
//      and .at() bounds checks (exceptions), [] doesn't
extern JsonObject WylesLibs::Json::parse(std::string * json) {
    if (json == nullptr) throw std::runtime_error("Invalid JSON string.");

    JsonObject root;
    JsonObject * obj = nullptr;
    size_t i = 0;
    char c = json->at(i);
    while(c != 0x00) {
        if (c == '{') {
            loggerPrintf(LOGGER_DEBUG, "Found %c @ %lu\n", c, i);
            // create new object... and update cursor/pointer to object.
            if (obj == nullptr) {
                obj = &root;
            } else {
                // lol?
                JsonValue * new_obj = (JsonValue *) new JsonObject();
                (obj->values).append(new_obj);

                // lol?
                obj = (JsonObject *) new_obj;
            }
            parseKey(obj, json, ++i);
            loggerPrintf(LOGGER_DEBUG, "New OBJ, @ %lu\n", i);
        } else if (c == ',') {
            loggerPrintf(LOGGER_DEBUG, "Found %c @ %lu\n", c, i);
            parseKey(obj, json, ++i);
        }
        c = json->at(++i);
        // loggerPrintf(LOGGER_DEBUG, "Found %c @ %lu\n", c, i);
        // printf("%c\n", c);
    }

    return root; // and so, when this is out of scope, deconstructors are called and things are cleaned up?
                    // or do I need to new/malloc?
}

