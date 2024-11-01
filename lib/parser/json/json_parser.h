#ifndef WYLESLIBS_JSON_PARSER_H
#define WYLESLIBS_JSON_PARSER_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "datastructures/array.h"
#include "estream/estream.h"
#include "file/file.h"

#include "global_consts.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

namespace WylesLibs::Parser::Json {

static constexpr size_t MAX_JSON_DEPTH = 7;
static constexpr size_t MAX_LENGTH_OF_JSON_STRING = 2<<27;
static constexpr size_t MAX_LENGTH_OF_JSON_STRING_KEYS = 2<<7;
static constexpr size_t MAX_LENGTH_OF_JSON_STRING_VALUES = 2<<19;

// Natural.Decimal<NUL>
static constexpr size_t JSON_NUMBER_MAX_DIGITS = NUMBER_MAX_DIGITS * 2 + 2;
// %NUM_MAX_DIGITS.NUM_MAX_DIGITSf<NUL>
static constexpr size_t JSON_NUMBER_FORMAT_STRING_SIZE = 4 + 2 + 2;

static constexpr size_t JSON_STRING_SIZE_LIMIT = 65536;

// single precision == exp is 8-bits... [-126, 127] (255, but centered around zero because decimal point can move in both directions.)
//  I am using double precision types (double) throughout the program so this shouldn't be an issue.
static constexpr size_t FLT_MAX_EXP_ABS = 127;

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
        virtual ~JsonBase() = default;
        virtual std::string toJsonElements() {
            return "";
        }
        virtual std::string toJsonString() {
            std::string s("{");
            s += toJsonElements();
            s += "}";
            return s;
        }
};

class JsonValue: public JsonBase {
    public:
        JsonType type;
        JsonValue(): type(NULL_TYPE) {}
        JsonValue(JsonType derived_type): type(derived_type) {}
        // TODO: odd syntax but...
        ~JsonValue() override = default;
        std::string toJsonString() {
            return "";
        }
};

typedef void(ProcessObjectFunc)(std::string key, JsonValue * value);
typedef void(ProcessValueFunc)(JsonValue * value);

class JsonBoolean: public JsonValue {
    private:
        bool boolean;
    public:
        JsonBoolean(bool boolean): boolean(boolean), JsonValue(BOOLEAN) {}
        ~JsonBoolean() override = default;

        bool getValue() {
            return this->boolean;
        }

        std::string toJsonString() final override {
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
        size_t natural_digit_count;
        size_t decimal_digit_count;
    public:
        JsonNumber(double number): number(number), natural_digit_count(SIZE_MAX), decimal_digit_count(SIZE_MAX), JsonValue(NUMBER) {}
        JsonNumber(double number, size_t natural_digit_count, size_t decimal_digit_count): number(number), natural_digit_count(natural_digit_count), decimal_digit_count(decimal_digit_count), JsonValue(NUMBER) {}
        ~JsonNumber() override = default;

        double getValue() {
            return this->number;
        }

        std::string toJsonString() final override {
            char format_i[JSON_NUMBER_FORMAT_STRING_SIZE] = {};
            if (natural_digit_count == SIZE_MAX || decimal_digit_count == SIZE_MAX) {
                sprintf(format_i, "%ld.%ldf", NUMBER_MAX_DIGITS, NUMBER_MAX_DIGITS);
            } else {
                sprintf(format_i, "%ld.%ldf", natural_digit_count, decimal_digit_count);
            }
            char format[JSON_NUMBER_FORMAT_STRING_SIZE] = {};
            format[0] = '%';
            strcat(format, format_i);
         
            char number[JSON_NUMBER_MAX_DIGITS] = {};
            sprintf(number, format, this->number);
            return std::string(number);
        }
};

class JsonString: public JsonValue {
    private:
        std::string s;
    public:
        JsonString(std::string s): s(s), JsonValue(STRING) {}
        ~JsonString() override = default;
        
        std::string getValue() {
            return this->s;
        }

        std::string toJsonString() final override {
            std::string ret("\"");
            ret += this->getValue();
            ret += "\"";

            return ret;
        }
};

// LOL, over optimizating? probably but this should definitely minimize memory footprint some... probably not as much as one would think though?
//  if nothing else, let's keep parsing stuff consistent... 
extern std::shared_ptr<JsonValue> parseFile(std::shared_ptr<StreamFactory> stream_factory, std::string file_path);
extern std::shared_ptr<JsonValue> parse(std::string json);
extern std::shared_ptr<JsonValue> parse(SharedArray<uint8_t> json);
extern std::shared_ptr<JsonValue> parse(ReaderEStream * r, size_t& i);

extern std::string pretty(std::string json);

}
#endif