#include <stdbool.h>
#include "json.h"

using namespace WylesLibs::Json;

size_t compareCString(const char * buf, const char * comp, size_t comp_length) {
    size_t x = 0;
    while(x < comp_length) {
        if (comp[x] != buf[i + x]) {
            break;
        }
        x++;
    }
    return x;
}

bool isHexDigit(char c) {
    return isDigit(c) || isLowerHex(c) || isUpperHex(c);
}

bool isDigit(char c) {
    if (c >= 0x30 && c <= 0x39) {
        return true;
    }
    return false;
}

bool isLowerHex(char c) {
    if (c >= 0x61 && c <= 0x66) {
        return true;
    }
    return false;
}

bool isUpperHex(char c) {
    if (c >= 0x41 && c <= 0x46) {
        return true;
    }
    return false;
}

char hexToChar(char * buf) {
    char ret = 0x00;
    for (size_t i = 0; i < 2; i++) {
        char c = buf[i];
        if (isDigit(c)) {
            c = c - 0x30;
        } else if (isLowerHex(c)) {
            c = c - 0x61;
        } else if (isUpperHex(c)) {
            c = c - 0x41;
        } else {
            // TODO:
            //  freak out! exception... something...
        }
        ret = ret << 4 | c;
    }
    return ret;
}

void parseDecimal(const char * buf, size_t& i, double& value) {
    double decimal_divisor = 10;
    char c = buf[i++];
    while (isDigit(c)) { // iterate non-digit. likely until comma, whitespace or exponential
        // 1.1234567
        // 1 + .1
        // 1.1 + .02
        value += (c - 0x30) / decimal_divisor;
        decimal_divisor *= 10;
        c = buf[i++];
    }
}

void parseNatural(const char * buf, size_t& i, double& value) {
    char c = buf[i++];
    while (isDigit(c)) { // iterate non-digit. likely until comma, whitespace or exponential
        // 1
        // 1 * 10 = 10 + 2;
        // 12 * 10 = 120 + 3; 
        value = (value * 10) + (c - 0x30); 
        c = buf[i++];
    }
}

void parseNumber(JsonObject * obj, const char * buf, size_t& i) {
    int8_t sign = buf[i] == '-' ? -1 : 1;

    int8_t exponential_sign = 0;
    double exponential_multiplier = 10;

    double value = 0;
    char c = buf[i++];

    // TODO: this is ugly...
    while (c != ',' && c != '}' && c != ' ' && c != '\r' && c != '\n' && c != '\t') {
        if (!isDigit(c)) {
            // throw exception/break, do something...?, move to while condition?
        } else if (c == '.') {
            parseDecimal(buf, value);
        } else if (c == 'e' || c == 'E') {
            c = buf[i++];
            exponential_sign = c == '-' ? -1 : 1;

            double exp = 0;
            parseNatural(buf, i, exp);

            // 1 = * 10
            // 2 = * 100
            // 3 = * 1000
            for (size_t x = 0; x < exp; x++) {
                exponential_multiplier *= 10;
            }
        } else { // parse number value...
            parseNatural(buf, i, value);
        }
        c = buf[i++];
    }

    if (exponential_sign == -1) {
        value = value / exponential_multiplier;
    } else if (exponential_sign == 1) {
        value = value * exponential_multiplier;
    }

    JsonNumber num(value * sign);
    (obj->values).append(num);
}

void parseString(JsonObject * obj, const char * buf, size_t& i) {
    char c = buf[++i];
    std::string s()
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
                s.append('\b');
            } else if (c == 'f') { // form feed
                s.append('\f');
            } else if (c == 'n') { // newline
                s.append('\n');
            } else if (c == 'r') { // carraige return
                s.append('\r');
            } else if (c == 't') { // tab
                s.append('\t')
            } else if (c == 'u') { // unicode
                for (size_t x = 0; x < 4; x = x + 2) {
                    // so, this takes a hex string and converts to it's binary value.
                    //  i.e. "0F" -> 0x0F;
                    s.append(hexToChar(buf + i + x));
                }
                // because indexing starts at 0 and x < 4 lol...
                i += 3
            } else {
                // actual characters can just be appended.
                s.append(c);
            }
        } else {
            s.append(c);
        }
        prev_c = c;
        c = buf[++i];
    }

    // lol... not the best but
    //  who cares rn ... value value value value
    JsonString jsonString(s);
    (obj->values).append(jsonString);
}

void parseArray(JsonObject * obj, const char * buf, size_t& i) {
    char c = buf[++i];
    while(c != ']') {
        // TODO: check if malformed array... and do something... bubble it up!
        // can either copy string and null terminate or pass delimeter to parseValue...
        parseValue(obj, buf + i, i, ',');
    }
}

void parseValue(JsonObject * obj, const char * buf, size_t& i, const char stop) {
    char c = buf[++i];
    while (c != stop) {
        // TODO:
        //  if's vs switches...?
        if (c == '"') {
            parseString(obj, buf + i, i);
            break;
        } else if (isDigit(c) || c == '+' || c = '-') {
            parseNumber(obj, buf + i, i);
            break;
        } else if (c == '[') {
            parseArray(obj, buf + i, i);
            break;
        } else if (c == 't') {
            const char * comp = "true";
            size_t consumed = compareCString(buf + i, comp, 4);
            if (consumed == 4) {
                JsonBoolean boolean(true);
                (obj->values).append(boolean);
            }
            i += consumed;
            break;
        } else if (c == 'f') {
            const char * comp = "false";
            size_t consumed = compareCString(buf + i, comp, 5);
            if (consumed == 5) {
                JsonBoolean boolean(false);
                (obj->values).append(boolean);
            }
            i += consumed;
            break;
        } else if (c == 'n') {
            const char * comp = "null";
            size_t consumed = compareCString(buf + i, comp, 4);
            if (consumed == 5) {
                JsonValue nullValue();
                (obj->values).append(nullValue);
            }
            i += consumed;
            break;
            // TODO:
            // hmm... this works but think about whether we want to just ignore malformed...
            //  when I think about validating input, ensureing full json, etc.
        } else if (c == '{') {
            break; // don't increment pointer and break;
        }
        // lol, completely ignored, true false and null... TODO:
        c = buf[++i];
    }
}

void parseKey(JsonObject * obj, const char * buf, size_t& i) {
    // find start and end index of key string
    // TODO:
    //  if empty string...
    //  already contains,,

    //  maybe allow empty string but enforce uniqueness...

    // might be worth implementing a map structure eventually.

    char c = buf[++i];
    while (c != '"') {
        c = buf[++i];
    } // found quote
    size_t start_i = ++i;

    char c = buf[i];
    while (c != '"') {
        c = buf[++i];
    } // found quote
    size_t end_i = --i;
    size_t size = end_i - start_i + 1; // inclusive
    if (size == 0) {

    }
    // implement array contains...
    (obj->keys).append(std::string(buf + start_i, size));
}

// TODO: need to validate input? make sure full json before this is called?
//  also, cleanup, be more consistent about how I increment the buffer (consume)
JsonObject parse(const char * json) {
    JsonObject root;
    JsonObject * obj = nullptr;
    size_t i = 0;
    char c = json[i];
    while(c != NULL) {
        if (c == '{') {
            // create new object... and update cursor/pointer to object.
            if (obj == nullptr) {
                obj = &root;
            } else {
                JsonObject new_obj;
                (obj->values).append(new_obj);

                // get pointer to newly appended obj...
                obj = (obj->values).buf + (obj->values).size-1;
            }
            parseKey(obj, json + i, i);
            // NULL check?
        } else if (c == ':') {
            // TODO: ws required? lmao
            parseValue(obj, json + i, i, (char)0x00);
            // NULL check?
        } else {
            c = json[i++];
        }
    }

    return root; // and so, when this is out of scope, deconstructors are called and things are cleaned up?
                    // or do I need to new/malloc?
}

