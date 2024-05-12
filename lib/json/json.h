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
}

class JsonValue {
    protected:
        JsonType type;
    public:
        JsonValue(): type(NULL_TYPE) {}
}

class JsonBoolean: JsonValue {
    public:
        bool boolean;
        JsonBoolean(bool boolean): boolean(boolean), type(BOOLEAN){}
}

class JsonNumber: JsonValue {
    public:
        double number;
        JsonNumber(double number): number(number), type(NUMBER){}
}

class JsonArray: JsonValue {
    public:
        WylesLibs::Array<JsonValue> values;
        JsonArray(WylesLibs::Array<JsonValue> values): values(values), type(ARRAY) {}
}

class JsonObject: JsonValue {
    public:
        WylesLibs::Array<std::string> keys;
        WylesLibs::Array<JsonValue> values;
        JsonObject(WylesLibs::Array<std::string> keys, WylesLibs::Array<JsonValue> values): type(OBJECT), keys(keys), values(values) {}
}

class JsonString: JsonValue {
    public:
        std::string s;
        JsonString(std::string s): type(STRING), s(s) {}
}

void parseString(JsonObject * obj, const char * buf, size_t& i);
void parseNumber(JsonObject * obj, const char * buf, size_t& i);
void parseArray(JsonObject * obj, const char * buf, size_t& i);
void parseValue(JsonObject * obj, const char * buf, size_t& i);
JsonObject parse(const char * json);

}
#endif