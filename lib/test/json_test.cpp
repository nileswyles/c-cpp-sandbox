#include "json.h"
#include <iostream>

#include "test/tester.h"

using namespace WylesLibs;

class Nested: public Json::JsonBase {
    public:
        std::string name;
        Nested * nested;
        Nested() {}
        Nested(Json::JsonObject * obj) {
            size_t validation_count = 0;
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                Json::JsonValue * value = obj->values.at(i);
                if(key == "nested_name") {
                    Json::setVariableFromJsonValue(value, name, validation_count);
                }
            }
            loggerPrintf(LOGGER_DEBUG, "validation count: %lu\n", validation_count);
            if (validation_count != obj->keys.size()) {
                throw std::runtime_error("Failed to create User from json object.");
            }
        }

        std::string toJsonString() {
            std::string s("{");
            s += "\"nested_name\": ";
            s += Json::JsonString(this->name).toJsonString();

            if (nested != nullptr) {
                s += ",\"nested\": ";
                s += nested->toJsonString();
            }

            s += "}";

            return s;
        }

        bool operator == (const Nested other) {
            if(this->name != other.name) {
                loggerPrintf(LOGGER_DEBUG, "name not equal\n");
                return false;
            }
            return true;
        }
};

class User: public Json::JsonBase {
    public:
        std::string name;
        std::string attributes;
        double dec;
        std::vector<bool> arr;
        Nested nested;

        // TODO: ?
        User() {}
        User(Json::JsonObject * obj) {
            size_t validation_count = 0;
            loggerPrintf(LOGGER_DEBUG, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG, "Key: %s\n", key.c_str());
                Json::JsonValue * value = obj->values.at(i);
                if(key == "name") {
                    Json::setVariableFromJsonValue(value, name, validation_count);
                } else if (key == "attributes") {
                    Json::setVariableFromJsonValue(value, attributes, validation_count);
                } else if (key == "dec") {
                    Json::setVariableFromJsonValue(value, dec, validation_count);
                } else if (key == "arr") {
                    // mixed type array...
                    std::vector<bool> * bool_arr = &arr;
                    std::vector<double> * num_arr = nullptr;
                    std::vector<std::string> * str_arr = nullptr;
                    // std::vector<bool> * bool_arr = &arr;

                    // womp womp womp....
                    //
                    Json::setArrayVariablesFromJsonValue<>(value, &arr, num_arr, str_arr, nullptr, validation_count);

                    // could be better...........
                    //  that said, object mappers for all strongly typed languages will have this limitation...
                    //  
                    //  good reason for nodejs? maybe web server using nodejs and implement IPC to do real work...?
                    //  
                    //      lol... downsides of IPC?

                    // if you know, single type array
                    // Json::setArrayVariablesFromJsonValue(value, arr, validation_count);
                } else if (key == "nested_obj") {
                    loggerPrintf(LOGGER_DEBUG, "Nested type.\n");
                    nested = Json::setVariableFromJsonValue<Nested>(value, validation_count);
                }
            }
            loggerPrintf(LOGGER_DEBUG, "validation count: %lu\n", validation_count);
            if (validation_count != obj->keys.size()) {
                throw std::runtime_error("Failed to create User from json object.");
            }
        }

        std::string toJsonString() {
            std::string s("{");
            s += "\"name\": ";
            s += Json::JsonString(this->name).toJsonString();
            s += ",";

            s += "\"attributes\": ";
            s += Json::JsonString(this->attributes).toJsonString();
            s += ",";

            s += "\"dec\": ";
            s += Json::JsonNumber(this->dec).toJsonString();
            s += ",";

            Json::JsonArray jsonArray = Json::JsonArray();
            for (size_t i = 0; i < arr.size(); i++) {
                Json::JsonBoolean * v = new Json::JsonBoolean(arr[i]);
                jsonArray.push_back(v);
            }
            s += "\"arr\": ";
            s += jsonArray.toJsonString();
            s += ",";

            s += "\"nested\": ";
            s += nested.toJsonString();
            s += "}";

            return s;
        }

        bool operator == (const User other) {
            if(this->name != other.name) {
                loggerPrintf(LOGGER_DEBUG, "name not equal\n");
                return false;
            }
            if(this->attributes != other.attributes) {
                loggerPrintf(LOGGER_DEBUG, "attributes not equal\n");
                return false;
            }
            // TODO: this needs to be revisited...
            if((int)(this->dec * 1000) != (int)(other.dec * 1000)) {
                loggerPrintf(LOGGER_DEBUG, "dec not equal\n");
                return false;
            }
            if(this->arr != other.arr) {
                loggerPrintf(LOGGER_DEBUG, "arr not equal\n");
                return false;
            }
            if(this->nested != other.nested) {
                loggerPrintf(LOGGER_DEBUG, "nested not equal\n");
                return false;
            }
            return true;
        }
};

void testJsonArray(void * test) {
    Test * t = (Test *)test;
    std::string s("[false, true, false, false]");
    std::vector<bool> expected{false, true, false, false};
    try {
        size_t i = 0;
        Json::JsonValue * obj = Json::parse(s, i);
        if (obj != nullptr) {
            if (obj->type == Json::ARRAY) {
                loggerPrintf(LOGGER_TEST, "JSON to Parse: \n");
                loggerPrintf(LOGGER_TEST, "%s\n", Json::pretty(s).c_str());
                Json::JsonArray * values = (Json::JsonArray *)obj;
                loggerPrintf(LOGGER_TEST, "Parsed JSON Array: \n");
                loggerPrintf(LOGGER_TEST, "%s\n", Json::pretty(values->toJsonString()).c_str());
                size_t validation_count = 0;
                for (size_t i = 0; i < values->size(); i++) {
                    Json::JsonValue * value = values->at(i);
                    if (value->type == Json::BOOLEAN) {
                        bool actual = ((Json::JsonBoolean *)value)->getValue();
                        if (expected[i] == actual) {
                            validation_count++;
                        }
                    }
                }
                if (validation_count == 4 && values->size() == 4) {
                    t->fail = false;
                }
            }
        }
        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
    }
}

std::string createNestedObject(size_t length) {
    Nested * root = new Nested();
    root->name = "username";
    Nested * prev = root;
    for (size_t i = 0; i < length; i++) {
        Nested * current = new Nested();
        current->name = "username";

        prev->nested = current; 
        prev = current;
    }
    // std::string s = Json::pretty(root->toJsonString());
    // loggerPrintf(LOGGER_DEBUG, "%s\n", s.c_str());
    // return s;
    return root->toJsonString();
}

std::string createNestedArray(size_t length) {
    Json::JsonArray * root = new Json::JsonArray();
    root->addValue((Json::JsonValue *)new Json::JsonBoolean(true));
    Json::JsonArray * prev = root;
    for (size_t i = 0; i < length; i++) {
        Json::JsonArray * current = new Json::JsonArray();
        current->addValue((Json::JsonValue *)new Json::JsonBoolean(true));

        prev->addValue(current);
        prev = current;
    }
    // std::string s = Json::pretty(root->toJsonString());
    // loggerPrintf(LOGGER_TEST, "%s\n", s.c_str());
    // return s;
    return root->toJsonString();
}

void parseObjectAndAssert(Test * t, std::string s, User expected, size_t expected_size) {
    try {
        size_t i = 0;
        Json::JsonObject * obj = (Json::JsonObject *)Json::parse(s, i);
        if (obj->type == Json::OBJECT) {
            User user(obj);
            loggerPrintf(LOGGER_TEST, "JSON To Parse: \n");
            loggerPrintf(LOGGER_TEST, "%s\n", Json::pretty(s).c_str());
            loggerPrintf(LOGGER_TEST, "Parsed JSON - User Class: \n");
            loggerPrintf(LOGGER_TEST, "%s\n", Json::pretty(user.toJsonString()).c_str());
            loggerPrintf(LOGGER_TEST, "Expected JSON - User Class: \n");
            loggerPrintf(LOGGER_TEST, "%s\n", Json::pretty(expected.toJsonString()).c_str());

            if (user == expected 
                && obj->keys.size() == expected_size 
                && obj->values.size() == expected_size) {
                t->fail = false;
            }
        }
        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
    }
}

void testJsonEmptyObject(void * test) {
    std::string s("{}");
    User expected;

    parseObjectAndAssert((Test *)test, s, expected, 0);
}

void testJsonObjectWithName(void * test) {
    std::string s("{ \n");
    s += "\"name\": \"username\", \n";
    s += "}\n";

    // TODO: hmm... let's think about this... not testing toStrings, so should I just use that?
    User expected;
    expected.name = "username";

    parseObjectAndAssert((Test *)test, s, expected, 1);
}

void testJsonObjectWithArray(void * test) {
    std::string s("{ \n");
    s += "\"arr\": [false, true, false, false], \n";
    s += "}\n";

    User expected;
    std::vector<bool> expected_arr{false, true, false, false};
    expected.arr = expected_arr;

    parseObjectAndAssert((Test *)test, s, expected, 1);
}

void testJson(void * test) {
    // Tester * t = (Tester *)tester;
    // std::string s("{ \n");
    // s += "\"name\":\"username\", \n";
    // s += "\"attributes\":\"attributes for user\", \n";
    // s += "\"arr\": [false, true, false, false], \n";
    // s += "\"dec\": 272727.1111, \n";
    // s += "\"nested_obj\": { \"nested_name\": \"nested_value\" }, \n";
    // s += "\"null_value\": null \n";
    // s += "}\n";

    Nested nested_obj;
    nested_obj.name = "nested_value";

    User expected;
    expected.name = "username";
    expected.attributes = "attributes for user";
    std::vector<bool> expected_arr{false, true, false, false};
    expected.arr = expected_arr;
    expected.dec = 272727.1111;
    expected.nested = nested_obj;

    parseObjectAndAssert((Test *)test, expected.toJsonString(), expected, 5);
}

void testJsonNestedObject(void * test) {
    std::string s = createNestedObject(2);
    try {
        size_t i = 0;
        Json::JsonObject * obj = (Json::JsonObject *)Json::parse(s, i);
        if (obj->type == Json::OBJECT) {
            // if (obj->toJsonString() == s 
            //     && obj->keys.size() == 77 * 2; 
            //     && obj->values.size() == 77 * 2) {

            // fprintf(obj->toJsonString() == s) {

            // lol.... nah.

            // TODO: write tests for the pretty function... because this relies on it.
            std::string actual = Json::pretty(obj->toJsonString());
            // printf("size: %u", obj->keys.size());
            std::string expected = Json::pretty(s.c_str());
            

            // printf("%s", actual.c_str());
            printf("Expected\n%s, %ld\n", expected.c_str(), expected.size());
            printf("Actual\n%s, %ld\n", actual.c_str(), actual.size());
            if (actual == expected) {
                ((Test *) test)->fail = false;
            }
        }
        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
    }
}

int main() {
    // TODO: test selection... from arguments?

    // std::string s("{\"test\":false, \"test2\":\"value\"}");
    // const char * s = "{\"test\":null, \"test2\":17272.2727}";
    Tester * t = tester_constructor(nullptr, nullptr, nullptr, nullptr);

    // tester_add_test(t, testJsonNestedArray);
    tester_add_test(t, testJsonNestedObject);

    // TODO: add {} test... lmao
    // tester_add_test(t, testJson);
    // tester_add_test(t, testJsonObjectWithArray);
    // tester_add_test(t, testJsonObjectWithName);
    // tester_add_test(t, testJsonArray);
    // tester_add_test(t, testJsonEmptyObject);
    tester_run(t);

    tester_destructor(t);

    return 0;
}