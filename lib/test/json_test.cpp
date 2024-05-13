#include "json.h"
#include <iostream>

using namespace WylesLibs::Json;

class User {
    public:
        std::string name;
        std::string attributes;

        User(JsonObject * obj) {
            throw std::runtime_error("Test exception");
            loggerPrintf(LOGGER_TEST, "TEST\n");
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                WylesLibs::Json::JsonValue * value = obj->values.at(i);
                WylesLibs::Json::JsonType type = value->type;
                loggerPrintf(LOGGER_TEST, "value type: %d\n", type);

                if(key == "name") {
                    // TODO:
                    //  better type conversion?
                    if (type == WylesLibs::Json::STRING) {
                        name = ((WylesLibs::Json::JsonString *)value)->getValue();
                    }
                } else if (key == "attributes") {
                    if (type == WylesLibs::Json::STRING) {
                        attributes = ((WylesLibs::Json::JsonString *)value)->getValue();
                    }
                }
            }

            // Okay, now incomplete values or of invalid type, what to do? throw exception? Constructor exceptions a thing?
            throw std::runtime_error("Test exception");
        }
};

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
    } else if (type == WylesLibs::Json::ARRAY) {


    }
}

int main() {
    // std::string s("{\"test\":false, \"test2\":\"value\"}");
    // const char * s = "{\"test\":null, \"test2\":17272.2727}";

    std::string s("{\"name\":\"username\", \"attributes\":\"attributes for user\"}");
    loggerPrintf(LOGGER_TEST, "JSON STRING: %s\n", s.c_str());
    try {
        // TODO:
        //  Think about this.... pass root as reference? parser class to manage new resources? or keep as is?
        JsonObject * obj = parse(&s);

        User user(obj);
        loggerPrintf(LOGGER_TEST, "%s\n", s.c_str());
        loggerPrintf(LOGGER_TEST, "%s, %s\n", user.name.c_str(), user.attributes.c_str());

        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
        //  throw e; // copy-initializes a new exception object of type std::exception
        // throw;   // rethrows the exception object of type std::out_of_range
    }
    return 0;
}