#ifndef WYLESLIBS_JSON_H
#define WYLESLIBS_JSON_H

#include "array.h"

namespace WylesLibs::Json {

typedef enum JsonType {
    BOOLEAN = 0,
    NUMBER,
    ARRAY,
    OBJECT,
    STRING,
    NULL_TYPE
} JsonType;

// TODO: fix names...
class JsonValue {
    public:
        JsonType type;
        JsonValue(): type(NULL_TYPE) {}
        JsonValue(JsonType derived_type): type(derived_type) {}
};

class JsonBoolean: public JsonValue {
    public:
        bool boolean;
        JsonBoolean(bool boolean): boolean(boolean), JsonValue(BOOLEAN) {}
};

class JsonNumber: public JsonValue {
    public:
        double number;
        JsonNumber(double number): number(number), JsonValue(NUMBER) {}
};

class JsonArray: public JsonValue {
    public:
        WylesLibs::Array<JsonValue *> values;
        JsonArray(WylesLibs::Array<JsonValue *> values): values(values), JsonValue(ARRAY) {}
};

class JsonObject: public JsonValue {
    public:
        WylesLibs::Array<std::string> keys;
        // this is hella lame...
        //  then destructor bs...
        //  whatever, onward...
        WylesLibs::Array<JsonValue *> values;
        JsonObject(): JsonValue(OBJECT) {} // lmao?
};

class JsonString: public JsonValue {
    public:
        std::string s;
        JsonString(std::string s): JsonValue(STRING), s(s) {}
};

JsonObject parse(const char * json);

//  TODO:
//      Function pointers a sin?
typedef void(ProcessValueFunc)(std::string key, JsonValue * value);

// TODO: abstract iterating too? iterators?... might be an array thing...
void processKeyValue(std::string key, JsonValue * value, ProcessValueFunc processor) {
    processor(key, value);
    // lmao, so lameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
    loggerPrintf(LOGGER_DEBUG, "MAKING SURE TO FREE POINTER! @ %p\n", value);
    delete value;
}

}
#endif