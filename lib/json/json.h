#pragma once
// lol

namespace WylesLibs {

typedef enum JsonType {
    NUMBER = 0,
    ARRAY,
    OBJECT,
    STRING
}

class JsonValue {
    JsonType type;
}

class JsonNumber: JsonValue {
    public:
        double number;
        JsonNumber(double number): number(number) {}
}

class JsonArray: JsonValue {
    public:
        Array<JsonValue> values;
        JsonArray(Array<JsonValue> values): values(values) {
            values = new Array<JsonValues>();
        }
        ~JsonArray() {
            delete values;
        }
}

class JsonObject: JsonValue {
    public:
        Array<std::string> keys;
        Array<JsonValue> values;
        JsonObject() {
            keys = new Array<std::string>();
            values = new Array<JsonValues>();
        }
        ~JsonObject() {
            delete keys;
            delete values;
        }
}

class JsonString: JsonValue {
    public:
        std::string s;
        JsonString(std::string s): s(s) {}
}

}