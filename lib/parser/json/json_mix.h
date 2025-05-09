#ifndef WYLESLIBS_JSON_MIX_H
#define WYLESLIBS_JSON_MIX_H

#include <string>

#include "parser/json/jstring.h"
#include "file/file.h"
#include "string_format.h"
#include "string_utils.h"
#include "global_consts.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

namespace WylesLibs::Parser::Json {

    static constexpr size_t MAX_JSON_DEPTH = 7;
    static std::string spacing[MAX_JSON_DEPTH] = {
        "\t",
        "\t\t",
        "\t\t\t",
        "\t\t\t\t",
        "\t\t\t\t\t",
        "\t\t\t\t\t\t",
        "\t\t\t\t\t\t\t"
    };
    static constexpr size_t MAX_LENGTH_OF_JSON_STRING = 2<<27;
    static constexpr size_t MAX_LENGTH_OF_JSON_STRING_KEYS = 2<<7;
    static constexpr size_t MAX_LENGTH_OF_JSON_STRING_VALUES = 2<<19;

    // Natural.Decimal<NUL>
    static constexpr size_t JSON_NUMBER_MAX_DIGITS = NUMBER_MAX_DIGITS * 2 + 2;
    // %NUM_MAX_DIGITS.NUM_MAX_DIGITSf<NUL>

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
            virtual std::string toJsonString() = 0;
    };

    // @ types representing the "intermediate" (strong-weak) datastructure.

    class JsonValue: public JsonBase {
        public:
            JsonType type;

            JsonValue(): type(NULL_TYPE) {}
            JsonValue(JsonType derived_type): type(derived_type) {
                loggerPrintf(LOGGER_DEBUG, "Json Type value defined: %d\n", type);
            }
            ~JsonValue() override = default;

            std::string toJsonString() {
                return "null";
            }

            template<typename T>
            T * as() {
                return (T *)this;
            }
    };

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

            JsonBoolean& operator= (bool value) {
                this->boolean = value;
                return *this;
            }

            bool operator== (bool value) {
                return this->boolean == value;
            }

            bool operator!= (bool value) {
                return this->boolean != value;
            }

            bool& operator* () {
                return this->boolean;
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
                // TODO: lol, hopefully this is portable?
                std::string format;
                if (natural_digit_count == SIZE_MAX || decimal_digit_count == SIZE_MAX) {
                    format = WylesLibs::format("%{d}.{d}f", NUMBER_MAX_DIGITS, NUMBER_MAX_DIGITS);
                } else {
                    format = WylesLibs::format("%{d}.{d}f", natural_digit_count, decimal_digit_count);
                }
                return WylesLibs::format(format, this->number);
            }

            JsonNumber& operator= (double value) {
                this->number = value;
                return *this;
            }
            JsonNumber& operator= (uint64_t value) {
                this->number = (double)value;
                return *this;
            }
            JsonNumber& operator= (int64_t value) {
                this->number = (double)value;
                return *this;
            }

            double operator+ (double value) {
                return number + (double)value;
            }
            double operator+ (uint64_t value) {
                return number + (double)value; // spicy - idk, might delete later.
            }
            double operator+ (int64_t value) {
                return number + (double)value;
            }

            double operator- (double value) {
                return number - (double)value;
            }
            double operator- (uint64_t value) {
                return number - (double)value;
            }
            double operator- (int64_t value) {
                return number - (double)value;
            }

            double operator* (double value) {
                return number * (double)value;
            }
            double operator* (uint64_t value) {
                return number * (double)value;
            }
            double operator* (int64_t value) {
                return number * (double)value;
            }

            double operator/ (double value) {
                return number / (double)value;
            }
            double operator/ (uint64_t value) {
                return number / (double)value;
            }
            double operator/ (int64_t value) {
                return number / (double)value;
            }

            bool operator== (JsonNumber& other) {
                return this->number == other.number;
            }

            bool operator== (double& value) {
                return this->number == value;
            }

            // TODO: ...
            double& operator* () {
                return this->number;
            }

    };

    class JsonString: public JsonValue {
        private:
            jstring s;

            std::string encodedValue() {
                #if defined(JSON_STRING_U32) || defined(JSON_STRING_U16) || defined(JSON_STRING_PLATFORM_STRING_UWP_CX) || defined(JSON_STRING_WSTRING)
                std::string utf8;
                for (auto c: this->s) {
                    if (c > 0x00FF) {
                        StringFormatOpts opts;
                        opts.base = 16;

                        #if defined(JSON_STRING_U32)
                        // TODO: right? unless already encoded in LE?
                        utf8 += "\\u" + numToString(c & 0x000000FF, opts) 
                            + numToString(c & 0x0000FF00 >> 8, opts)
                            + numToString(c & 0x00FF0000 >> 16, opts)
                            + numToString(c & 0xFF000000 >> 24, opts);
                        #elif defined(JSON_STRING_U16) || defined(JSON_STRING_PLATFORM_STRING_UWP_CX) || defined(JSON_STRING_WSTRING)
                        // assuming LE because duh? somewhat risky/unknown but not as bad as 
                        utf8 += "\\u0000" + numToString(c & 0x000000FF, opts) 
                            + numToString(c & 0x0000FF00 >> 8, opts);
                        #endif
                    } else {
                        utf8 += (char)c;
                    }
                }
                return utf8;
                #else
                return this->getValue();
                #endif
            }
        public:
            JsonString(jstring s): s(s), JsonValue(STRING) {}
            ~JsonString() override = default;

            jstring getValue() {
                return this->s;
            }

            std::string toJsonString() final override {
                std::string ret("\"");

                ret += encodedValue();
                ret += "\"";

                return ret;
            }

            JsonString& operator= (jstring other) {
                this->s = other;
                return *this;
            }

            jstring& operator* () {
                return this->s;
            }
    };

    // JsonArray
    // JsonObject
}
#endif