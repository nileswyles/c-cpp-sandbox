#include "json.h"
#include <iostream>

using namespace WylesLibs::Json;

// lol
void printProcessFunc(std::string key, WylesLibs::Json::JsonValue * value) {
    WylesLibs::Json::JsonType type = value->type;
    loggerPrintf(LOGGER_TEST, "value type: %d\n", type);

    if (type == WylesLibs::Json::BOOLEAN) {
        WylesLibs::Json::JsonBoolean * booleanValue = (JsonBoolean *)value;
        loggerPrintf(LOGGER_TEST, "boolean value: %u\n", booleanValue->getValue());
    } else if (type == WylesLibs::Json::STRING) {
        // when created, sizeof(JsonString) on stack? or sizeof(JsonValue)? lol 
        loggerPrintf(LOGGER_TEST, "lol?\n");
        WylesLibs::Json::JsonString * stringValue = (JsonString *)value;
        loggerPrintf(LOGGER_TEST, "string value: %s\n", stringValue->getValue().c_str());
    } else if (type == WylesLibs::Json::NUMBER) {
        // when created, sizeof(JsonString) on stack? or sizeof(JsonValue)? lol 
        loggerPrintf(LOGGER_TEST, "lol....\n");
        WylesLibs::Json::JsonNumber * numberValue = (JsonNumber *)value;
        loggerPrintf(LOGGER_TEST, "number value: %f\n", numberValue->getValue());

        if (key == "test2") {
            // blah blah blah, add populate class membh
            // this->test2 = (cast_type)numberValue->value;
        }
    }
}

int main() {
    std::string s("{\"test\":false, \"test2\":\"value\"}");

    // const char * s = "{\"test\":null, \"test2\":17272.2727}";

    loggerPrintf(LOGGER_TEST, "JSON STRING: %s\n", s.c_str());

    try {
        JsonObject obj = parse(&s);
        // CPP is beautiful! might not be the most performant though... lol
        for (auto i: obj.keys) {
            JsonObject::processValue(i, obj.values.at(i), printProcessFunc);
        }
        // if already freed, then do nothing? lol is that not how free works?
        loggerPrintf(LOGGER_TEST, "...\n");
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
        //  throw e; // copy-initializes a new exception object of type std::exception
        // throw;   // rethrows the exception object of type std::out_of_range
    }

    return 0;
}