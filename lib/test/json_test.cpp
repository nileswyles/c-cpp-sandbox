#include "json.h"
#include <iostream>

#include "test/tester.h"

using namespace WylesLibs::Json;

class Nested {
    public:
        std::string nested_name;
};

class User {
    public:
        std::string name;
        std::string attributes;
        double dec;
        std::vector<bool> arr;
        Nested nested;

        User(JsonObject * obj) {
            size_t values_set = 0;
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                WylesLibs::Json::JsonValue * value = obj->values.at(i);
                WylesLibs::Json::JsonType type = value->type;
                loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);

                if(key == "name") {
                    // TODO:
                    //  better type conversion?
                    if (type == WylesLibs::Json::STRING) {
                        name = ((WylesLibs::Json::JsonString *)value)->getValue();
                        values_set++;
                    }
                } else if (key == "attributes") {
                    if (type == WylesLibs::Json::STRING) {
                        attributes = ((WylesLibs::Json::JsonString *)value)->getValue();
                        values_set++;
                    }
                }
            }
            if (values_set != 2) {
                throw std::runtime_error("Failed to create User from json object.");
            }
        }
};

void testJson(void * tester) {
    Tester * t = (Tester *)tester;
    // lol
    std::string s("{ ");
    s += "\"name\":\"username\", ";
    s += "\"attributes\":\"attributes for user\", ";
    s += "\"arr\": [false, true, false, false], ";
    s += "\"dec\": 272727.1111, ";
    s += "\"nested_obj\": { \"nested_name\": \"nested_value\" }, ";
    s += "\"null_value\": null ";
    s += "}";

    try {
        // TODO:
        //  Think about this.... pass root as reference? parser class to manage new resources? or keep as is?
        JsonObject * obj = parse(&s);

        User user(obj);
        loggerPrintf(LOGGER_TEST, "JSON: \n");
        loggerPrintf(LOGGER_TEST, "  %s\n", s.c_str());
        loggerPrintf(LOGGER_TEST, "User Class: \n");
        loggerPrintf(LOGGER_TEST, "  %s, %s\n", user.name.c_str(), user.attributes.c_str());

        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
        //  throw e; // copy-initializes a new exception object of type std::exception
        // throw;   // rethrows the exception object of type std::out_of_range
    }
}

int main() {
    // std::string s("{\"test\":false, \"test2\":\"value\"}");
    // const char * s = "{\"test\":null, \"test2\":17272.2727}";
    Tester * t = tester_constructor(nullptr, nullptr, nullptr, nullptr);

    tester_add_test(t, testJson);
    tester_run(t);

    tester_destructor(t);

    return 0;
}