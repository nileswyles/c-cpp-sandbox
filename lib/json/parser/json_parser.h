#ifndef WYLESLIBS_JSON_PARSER_H
#define WYLESLIBS_JSON_PARSER_H

#include "logger.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace WylesLibs::Json {

static constexpr size_t MAX_JSON_DEPTH = 7;
static constexpr size_t MAX_LENGTH_OF_JSON_STRING = 2<<27;
static constexpr size_t MAX_LENGTH_OF_JSON_STRING_KEYS = 2<<7;
static constexpr size_t MAX_LENGTH_OF_JSON_STRING_VALUES = 2<<19;

// single precision == exp is 8-bits... [-126, 127] (255, but centered around zero because decimal point can move in both directions.)
//  I am using double precision types (double) throughout the program so this shouldn't be an issue.
static constexpr size_t FLT_MAX_EXP_ABS = 127;
// this is still an arbitrary limit chosen based on number of digits of 2**32.
static constexpr size_t FLT_MAX_MIN_DIGITS = 10;

typedef enum JsonType {
    NULL_TYPE = 0,
    BOOLEAN,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT,
} JsonType;

class JsonBase {
    public:
        // TODO:
        //  Doesn't make much sense to me... but need to declare virtual destructor in base classes as such for derived destructors to be called.
        //  Revisit this... 
        virtual ~JsonBase() {};
        virtual std::string toJsonString() = 0;
};

class JsonValue: public JsonBase {
    public:
        JsonType type;
        JsonValue(): type(NULL_TYPE) {}
        JsonValue(JsonType derived_type): type(derived_type) {}
        virtual ~JsonValue() {}
        virtual std::string toJsonString() = 0;
};

typedef void(ProcessObjectFunc)(std::string key, JsonValue * value);
typedef void(ProcessValueFunc)(JsonValue * value);

class JsonBoolean: public JsonValue {
    private:
        bool boolean;
    public:
        JsonBoolean(bool boolean): boolean(boolean), JsonValue(BOOLEAN) {}

        bool getValue() {
            return this->boolean;
        }

        std::string toJsonString() {
            if (boolean) {
                return std::string("true");
            } else {
                return std::string("false");
            }
        }
};

class JsonNumber: public JsonValue {
    private:
        double number;
    public:
        JsonNumber(double number): number(number), JsonValue(NUMBER) {}

        double getValue() {
            return this->number;
        }

        std::string toJsonString() {
            // TODO: when figure out limits (precision), adjust this...
            char number_arr[16];
            sprintf(number_arr, "%f", this->number);
            return std::string(number_arr);
        }
};

class JsonString: public JsonValue {
    private:
        std::string s;
    public:
        JsonString(std::string s): s(s), JsonValue(STRING) {}
        
        std::string getValue() {
            return this->s;
        }

        std::string toJsonString() {
            std::string ret("\"");
            ret += this->getValue();
            ret += "\"";

            return ret;
        }
};

extern JsonValue * parse(std::string json, size_t& i);
extern std::string pretty(std::string json);

}
#endif