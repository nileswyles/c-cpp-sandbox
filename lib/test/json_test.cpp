#include "json.h"
#include <iostream>

#include "test/tester.h"

#ifndef LOGGER_JSON_TEST
#define LOGGER_JSON_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_TEST
#include "logger.h"

using namespace WylesLibs;

class Nested: public Json::JsonBase {
    public:

        std::string name;
        Nested * nested;
        Nested(): nested(nullptr) {}
        Nested(Json::JsonObject * obj): nested(nullptr) {
            size_t validation_count = 0;
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                Json::JsonValue * value = obj->values.at(i);
                if(key == "nested_name") {
                    name = Json::setVariableFromJsonValue<std::string>(value, validation_count);
                }
            }
            loggerPrintf(LOGGER_TEST_VERBOSE, "validation count: %lu\n", validation_count);
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
                loggerPrintf(LOGGER_TEST_VERBOSE, "name not equal\n");
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
            loggerPrintf(LOGGER_TEST_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_TEST_VERBOSE, "Key: %s\n", key.c_str());
                Json::JsonValue * value = obj->values.at(i);
                if(key == "name") {
                    name = Json::setVariableFromJsonValue<std::string>(value, validation_count);
                } else if (key == "attributes") {
                    attributes = Json::setVariableFromJsonValue<std::string>(value, validation_count);
                } else if (key == "dec") {
                    dec = Json::setVariableFromJsonValue<double>(value, validation_count);
                } else if (key == "arr") {
                    Json::setArrayVariablesFromJsonValue<bool>(value, arr, validation_count);
                } else if (key == "nested") {
                    loggerPrintf(LOGGER_TEST_VERBOSE, "Nested type.\n");
                    nested = Json::setVariableFromJsonValue<Nested>(value, validation_count);
                }
            }
            loggerPrintf(LOGGER_TEST_VERBOSE, "validation count: %lu\n", validation_count);
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
                loggerPrintf(LOGGER_TEST_VERBOSE, "name not equal\n");
                return false;
            }
            if(this->attributes != other.attributes) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "attributes not equal\n");
                return false;
            }
            // TODO: this needs to be revisited...
            if((int)(this->dec * 1000) != (int)(other.dec * 1000)) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "dec not equal\n");
                return false;
            }
            if(this->arr != other.arr) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "arr not equal\n");
                return false;
            }
            if(this->nested != other.nested) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "nested not equal\n");
                return false;
            }
            return true;
        }
};

static std::string createNestedObject(size_t length) {
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
    // loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", s.c_str());
    // return s;
    return root->toJsonString();
}

static std::string createNestedArray(size_t length) {
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

static void parseObjectAndAssert(TestArg * t, std::string s, User expected, size_t expected_size) {
    try {
        size_t i = 0;
        Json::JsonObject * obj = (Json::JsonObject *)Json::parse(s, i);
        if (obj->type == Json::OBJECT) {
            User user(obj);
            loggerPrintf(LOGGER_TEST_VERBOSE, "JSON To Parse: \n");
            loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", Json::pretty(s).c_str());
            loggerPrintf(LOGGER_TEST_VERBOSE, "Parsed JSON - User Class: \n");
            loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", Json::pretty(user.toJsonString()).c_str());
            loggerPrintf(LOGGER_TEST_VERBOSE, "Expected JSON - User Class: \n");
            loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", Json::pretty(expected.toJsonString()).c_str());

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

static void parseObjectAndAssertStringComparison(TestArg * t, std::string s) {
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
            loggerPrintf(LOGGER_TEST_VERBOSE, "Expected\n%s, %ld\n", expected.c_str(), expected.size());
            loggerPrintf(LOGGER_TEST_VERBOSE, "Actual\n%s, %ld\n", actual.c_str(), actual.size());
            if (actual == expected) {
                t->fail = false;
            }
        }
        delete obj;
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
    }

}

static void testJsonArray(TestArg * t);
static void testJsonNestedObject(TestArg * t);
static void testJsonEmptyObject(TestArg * t);
static void testJsonObjectWithName(TestArg * t);
static void testJsonObjectWithArray(TestArg * t);
static void testJson(TestArg * t);

static void testPretty(TestArg * t);

int main(int argc, char * argv[]) {
    Tester * t = tester_constructor(nullptr, nullptr, nullptr, nullptr);

    tester_add_test(t, testJsonNestedObject);
    // tester_add_test(t, testJsonNestedArray);
    tester_add_test(t, testJsonArray);
    tester_add_test(t, testJsonEmptyObject);
    tester_add_test(t, testJsonObjectWithName);
    tester_add_test(t, testJsonObjectWithArray);
    tester_add_test(t, testJson);

    tester_add_test(t, testPretty);

    // testMalformedJson
    //  -

    // specifically test mapper stuff...
    
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        tester_run(t, argv[1]);
    } else {
        tester_run(t, NULL);
    }

    tester_destructor(t);

    return 0;
}

static void testJsonArray(TestArg * t) {
    std::string s("[false, true, false, false]");
    std::vector<bool> expected{false, true, false, false};
    try {
        size_t i = 0;
        Json::JsonValue * obj = Json::parse(s, i);
        if (obj != nullptr) {
            if (obj->type == Json::ARRAY) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "JSON to Parse: \n");
                loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", Json::pretty(s).c_str());
                Json::JsonArray * values = (Json::JsonArray *)obj;
                loggerPrintf(LOGGER_TEST_VERBOSE, "Parsed JSON Array: \n");
                loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", Json::pretty(values->toJsonString()).c_str());
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

static void testJsonNestedObject(TestArg * t) {
    std::string s = createNestedObject(2);

    parseObjectAndAssertStringComparison(t, s);
}


static void testJsonEmptyObject(TestArg * t) {
    std::string s("{}");
    User expected;

    parseObjectAndAssert(t, s, expected, 0);
}

static void testJsonObjectWithName(TestArg * t) {
    std::string s("{ \n");
    s += "\"name\": \"username\", \n";
    s += "}\n";

    // TODO: hmm... let's think about this... not testing toStrings, so should I just use that?
    User expected;
    expected.name = "username";

    parseObjectAndAssert(t, s, expected, 1);
}

static void testJsonObjectWithArray(TestArg * t) {
    std::string s("{ \n");
    s += "\"arr\": [false, true, false, false], \n";
    s += "}\n";

    User expected;
    std::vector<bool> expected_arr{false, true, false, false};
    expected.arr = expected_arr;

    parseObjectAndAssert(t, s, expected, 1);
}

static void testJson(TestArg * t) {
    std::string s("{ \n");
    s += "\"name\":\"username\", \n";
    s += "\"attributes\":\"attributes for user\", \n";
    s += "\"arr\": [false, true, false, false], \n";
    s += "\"dec\": 272727.1111, \n";
    s += "\"nested\": { \"nested_name\": \"nested_value\" }, \n";
    s += "\"null_value\": null \n";
    s += "}\n";

    Nested nested_obj;
    nested_obj.name = "nested_value";

    User expected;
    expected.name = "username";
    expected.attributes = "attributes for user";
    std::vector<bool> expected_arr{false, true, false, false};
    expected.arr = expected_arr;
    expected.dec = 272727.1111;
    expected.nested = nested_obj;

    parseObjectAndAssert(t, expected.toJsonString(), expected, 5);
}

static void testPretty(TestArg * t) {
    std::string s("{ ");
    s += "\"name\":\"username\", ";
    s += "\"attributes\":\"attributes for user\", ";
    s += "\"arr\": [false, true, false, false], ";
    s += "\"dec\": 272727.1111, ";
    s += "\"nested\": { \"nested_name\": \"nested_value\" }, ";
    s += "\"null_value\": null ";
    s += "}";
    std::string actual = Json::pretty(s);

    /*
        {
            "name": "username",
            "attributes": "attributes for user",
            "arr": [
                    false,
                    true,
                    false,
                    false
            ],
            "dec": 272727.1111,
            "nested": {
                    "nestedname": "nestedvalue"
            },
            "nullvalue": null
        }
    */
    std::string expected("{\n");
    expected += "\t\"name\": \"username\",\n";
    expected += "\t\"attributes\": \"attributes for user\",\n";
    expected += "\t\"arr\": [\n";
    expected += "\t\tfalse,\n";
    expected += "\t\ttrue,\n";
    expected += "\t\tfalse,\n";
    expected += "\t\tfalse\n";
    expected += "\t],\n";
    expected += "\t\"dec\": 272727.1111,\n";
    expected += "\t\"nested\": {\n";
    expected += "\t\t\"nestedname\": \"nestedvalue\"\n";
    expected += "\t},\n";
    expected += "\t\"nullvalue\": null\n";
    expected += "}";

    loggerPrintf(LOGGER_TEST_VERBOSE, "Expected\n%s, %ld\n", expected.c_str(), expected.size());
    loggerPrintf(LOGGER_TEST_VERBOSE, "Actual\n%s, %ld\n", actual.c_str(), actual.size());

    for (size_t i = 0; i < expected.size(); i++) {
        if (actual.size() > i) {
            if (expected[i] != actual[i]) {
                loggerPrintf(LOGGER_TEST, "Mismatched character @ %lu: \n", i);
                loggerPrintf(LOGGER_TEST, "Expected: '%c', Actual: '%c'\n", expected[i], actual[i]);
            }
        } else {
            loggerPrintf(LOGGER_TEST, "Mismatched character @ %lu: \n", i);
            loggerPrintf(LOGGER_TEST, "Expected: '%c', Actual: ''\n", expected[i]);
        }
    }

    if (actual == expected) {
        t->fail = false;
    }
}