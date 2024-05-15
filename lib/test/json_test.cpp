#include "json.h"
#include <iostream>

#include "test/tester.h"

using namespace WylesLibs;

class Nested: public Json::JsonBase {
    public:
        std::string nested_name;
        Nested() {} // ?
        Nested(Json::JsonObject * obj) {
            // TODO: nullptr
            size_t validation_count = 0;
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                Json::JsonValue * value = obj->values.at(i);
                Json::JsonType type = value->type;
                loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
                if(key == "nested_name") {
                    Json::setVariableFromJsonValue(type, value, nested_name, validation_count);
                }
            }
            size_t expected_values = 1;
            loggerPrintf(LOGGER_DEBUG, "validation count: %lu\n", validation_count);
            if (validation_count != expected_values) {
                throw std::runtime_error("Failed to create User from json object.");
            }
        }

        std::string toJsonString() {
            std::string s("{");
            s += "\"nested_name\": ";
            s += "\"";
            s += this->nested_name;
            s += "\"";
            s += "}";

            return Json::pretty(s);
        }
};

class User: public Json::JsonBase {
    public:
        std::string name;
        std::string attributes;
        double dec;
        std::vector<bool> arr;
        Nested nested;

        User(Json::JsonObject * obj) {
            // TODO: nullptr
            size_t validation_count = 0;
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                Json::JsonValue * value = obj->values.at(i);
                Json::JsonType type = value->type;
                loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
                if(key == "name") {
                    Json::setVariableFromJsonValue(type, value, name, validation_count);
                } else if (key == "attributes") {
                    Json::setVariableFromJsonValue(type, value, attributes, validation_count);
                } else if (key == "dec") {
                    Json::setVariableFromJsonValue(type, value, dec, validation_count);
                } else if (key == "arr") {
                    Json::setArrayVariablesFromJsonValue(type, value, arr, validation_count);
                } else if (key == "nested") {
                    nested = Json::setVariableFromJsonValue<Nested>(type, value, validation_count);
                    // TODO: object constructors not called yet?
                    //  reference of not yet consructed items? 
                }
            }
            size_t expected_values = 3;
            loggerPrintf(LOGGER_DEBUG, "validation count: %lu\n", validation_count);
            if (validation_count != expected_values) {
                throw std::runtime_error("Failed to create User from json object.");
            }
        }

        std::string toJsonString() {
            char dec_arr[16];
            sprintf(dec_arr, "%f", this->dec);
            std::string dec_string(dec_arr);

            std::string s("{");
            s += "\"name\": ";
            s += "\"";
            s += this->name;
            s += "\"";
            s += ",";

            s += "\"attributes\": ";
            s += "\"";
            s += this->attributes;
            s += "\"";
            s += ",";

            s += "\"dec\": ";
            s += "\"";
            s += dec_string;
            s += "\"";
            s += ",";

            s += "\"nested\": ";
            s += nested.toJsonString();
            s += "}";

            // TODO:
            //  default to this?
            return Json::pretty(s);
        }
};

void testJsonArray(void * tester) {
    std::string s("[false, true, false, false]");
    try {
        // TODO:
        //  Think about this.... pass root as reference? parser class to manage new resources? or keep as is?
        Json::JsonValue * obj = Json::parse(s);
        if (obj->type == Json::ARRAY) {
            loggerPrintf(LOGGER_TEST, "JSON: \n");
            loggerPrintf(LOGGER_TEST, "  %s\n", Json::pretty(s).c_str());
            Json::JsonArray * values = (Json::JsonArray *)obj;
            for (size_t i = 0; i < values->size(); i++) {
                loggerPrintf(LOGGER_TEST, "User Class: \n");
                loggerPrintf(LOGGER_TEST, "%u\n", ((Json::JsonBoolean *)values->at(i))->getValue());
            }
        } else {
            // something went terribly wrong
        }
        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
        //  throw e; // copy-initializes a new exception object of type std::exception
        // throw;   // rethrows the exception object of type std::out_of_range
    }
}

void testJsonObjectWithArray(void * tester) {
    std::string s("{ \n");
    s += "\"arr\": [false, true, false, false], \n";
    s += "}\n";

    try {
        // TODO:
        //  Think about this.... pass root as reference? parser class to manage new resources? or keep as is?
        Json::JsonValue * obj = Json::parse(s);
        if (obj->type == Json::OBJECT) {
            User user((Json::JsonObject *)obj);
            loggerPrintf(LOGGER_TEST, "JSON: \n");
            loggerPrintf(LOGGER_TEST, "  %s\n", Json::pretty(s).c_str());
            loggerPrintf(LOGGER_TEST, "User Class: \n");
            loggerPrintf(LOGGER_TEST, "%s\n", user.toJsonString().c_str());
        } else {
            // something went terribly wrong
        }
        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
        //  throw e; // copy-initializes a new exception object of type std::exception
        // throw;   // rethrows the exception object of type std::out_of_range
    }
}

void testJson(void * tester) {
    Tester * t = (Tester *)tester;
    // lol
    std::string s("{ \n");
    s += "\"name\":\"username\", \n";
    s += "\"attributes\":\"attributes for user\", \n";
    // s += "\"arr\": [false, true, false, false], \n";
    s += "\"dec\": 272727.1111, \n";
    s += "\"nested_obj\": { \"nested_name\": \"nested_value\" }, \n";
    s += "\"null_value\": null \n";
    s += "}\n";

    try {
        // TODO:
        //  Think about this.... pass root as reference? parser class to manage new resources? or keep as is?
        Json::JsonValue * obj = Json::parse(s);
        if (obj->type == Json::OBJECT) {
            User user((Json::JsonObject *)obj);
            loggerPrintf(LOGGER_TEST, "JSON: \n");
            loggerPrintf(LOGGER_TEST, "  %s\n", Json::pretty(s).c_str());
            loggerPrintf(LOGGER_TEST, "User Class: \n");
            loggerPrintf(LOGGER_TEST, "%s\n", user.toJsonString().c_str());
        } else {
            // something went terribly wrong
        }
        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
        //  throw e; // copy-initializes a new exception object of type std::exception
        // throw;   // rethrows the exception object of type std::out_of_range
    }
}

int main() {
    // TODO: test selection... from arguments?

    // std::string s("{\"test\":false, \"test2\":\"value\"}");
    // const char * s = "{\"test\":null, \"test2\":17272.2727}";
    Tester * t = tester_constructor(nullptr, nullptr, nullptr, nullptr);

    tester_add_test(t, testJson);
    // tester_add_test(t, testJsonObjectWithArray);
    // tester_add_test(t, testJsonArray);
    tester_run(t);

    tester_destructor(t);

    return 0;
}