#ifndef WYLESLIBS_JSON_H
#define WYLESLIBS_JSON_H

#include "array.h"

namespace WylesLibs::Json {

typedef void(ProcessObjectFunc)(std::string key, JsonValue * value);
typedef void(ProcessValueFunc)(JsonValue * value);

typedef enum JsonType {
    NULL_TYPE = 0,
    BOOLEAN,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT,
} JsonType;

class JsonValue {
    public:
        JsonType type;
        JsonValue(): type(NULL_TYPE) {}
        JsonValue(JsonType derived_type): type(derived_type) {}
};

class JsonBoolean: public JsonValue {
    private:
        bool boolean;
    public:
        JsonBoolean(bool boolean): boolean(boolean), JsonValue(BOOLEAN) {}

        bool getValue() {
            return this->boolean;
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
};

class JsonString: public JsonValue {
    private:
        std::string s;
    public:
        JsonString(std::string s): s(s), JsonValue(STRING) {}
        
        std::string getValue() {
            return this->s;
        }
};

// TODO: move JsonArray and JsonObject defintion/implementation to their own .h and .cpp files...
class JsonArray: public JsonValue, public std::vector<JsonValue *> {
    public:
        JsonArray(): JsonValue(ARRAY), std::vector<JsonValue *>() {}

        static void processValue(JsonValue * value, ProcessValueFunc processor) {
            processor(value);
            // lmao, so lameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
            loggerPrintf(LOGGER_DEBUG, "MAKING SURE TO FREE POINTER! @ %p\n", value);
            delete value;
        }
};

// yeah, definetly use associative arrays over regular arrays when technically useful.
//  but in this case, might not be technially useful lol....
class JsonObjectEntry {
    public:
        std::string key;
        JsonValue * value;
}

class JsonObject: public JsonValue, public std::vector<JsonObjectEntry> {
    public:
        JsonObject(): JsonValue(OBJECT), std::vector<JsonObjectEntry>() {}

        void addKey(std::string key) {
            JsonObjectEntry e(key, nullptr);
            this->push_back(e);
        }
        void addValue(JsonValue * value) {
            this->back().value = value;
        }
        static void processValue(std::string key, JsonValue * value, ProcessObjectFunc processor) {
            processor(key, value);
            // lmao, so lameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
            loggerPrintf(LOGGER_DEBUG, "MAKING SURE TO FREE POINTER! @ %p\n", value);
            delete value;
        }
};

extern JsonObject parse(std::string * json);

}
#endif