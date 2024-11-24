#include "parser/json/json.h"
#include <iostream>
#include <memory>
#include "eshared_ptr.h"

// #include "test/tester.h"
#include "tester.h"

#ifndef LOGGER_JSON_TEST
#define LOGGER_JSON_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;
using namespace WylesLibs::Parser::Json;

class Nested: public JsonBase {
    public:
        std::string name;
        Nested * nested;
        // Nested() = default;
        Nested(): name(""), nested(nullptr) {}
        Nested(ESharedPtr<JsonObject> obj_shared): Nested() {
            JsonObject * obj = ESHAREDPTR_GET_PTR(obj_shared);
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                JsonValue * value = obj->values.at(i);
                if(key == "nested_name") {
                    name = setVariableFromJsonValue<std::string>(value);
                }
            }
        }
        ~Nested() = default;

        std::string toJsonString() {
            std::string s("{");
            s += "\"nested_name\": ";
            s += JsonString(this->name).toJsonString();

            if (nested != nullptr) {
                s += ",\"nested\": ";
                s += nested->toJsonString();
            }

            s += "}";

            return s;
        }

        bool operator ==(const Nested& other) {
            if(this->name != other.name) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "name not equal\n");
                return false;
            }
            return true;
        }
        bool operator !=(const Nested& other) {
            return !(*this == other);
        }
};

class User: public JsonBase {
    public:
        std::string name;
        std::string attributes;
        double dec;
        std::vector<bool> arr;
        Nested nested;

        // User() = default;
        User(): name(""), attributes(""), dec(0), arr(std::vector<bool>()), nested(Nested()) {}
        // TODO: specify required fields
        User(ESharedPtr<JsonObject> obj_shared): User() {
            JsonObject * obj = ESHAREDPTR_GET_PTR(obj_shared);
            loggerPrintf(LOGGER_TEST_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_TEST_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = obj->values.at(i);
                if (key == "name") {
                    name = setVariableFromJsonValue<std::string>(value);
                } else if (key == "attributes") {
                    attributes = setVariableFromJsonValue<std::string>(value);
                } else if (key == "dec") {
                    dec = setVariableFromJsonValue<double>(value);
                } else if (key == "arr") {
                    setArrayVariablesFromJsonValue<bool>(value, arr);
                } else if (key == "null_value") {
                    loggerPrintf(LOGGER_TEST_VERBOSE, "Null Value.\n");
                    // let's allow null values and handle during mapping and validation.
                } else if (key == "nested") {
                    loggerPrintf(LOGGER_TEST_VERBOSE, "Nested type.\n");
                    nested = setVariableFromJsonValue<Nested>(value);
                    // setVariableFromJsonValue<Nested>(value, nested);
                }
            }
        }
        ~User() = default;

        std::string toJsonString() {
            std::string s("{");
            s += "\"name\": ";
            s += JsonString(this->name).toJsonString();
            s += ",";

            s += "\"attributes\": ";
            s += JsonString(this->attributes).toJsonString();
            s += ",";

            s += "\"dec\": ";
            s += JsonNumber(this->dec).toJsonString();
            s += ",";

            JsonArray jsonArray = JsonArray();
            for (size_t i = 0; i < arr.size(); i++) {
                JsonBoolean * v = new JsonBoolean(arr[i]);
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

        bool operator ==(const User& other) {
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
            // TODO:
            //  lol... so, you don't also need to define != operator?
            if(this->nested != other.nested) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "nested not equal\n");
                return false;
            }
            return true;
        }

        bool operator !=(const User& other) {
            return !(*this == other);
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
    // std::string s = pretty(root->toJsonString());
    // loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", s.c_str());
    // return s;
    return root->toJsonString();
}

static std::string createNestedArray(size_t length) {
    JsonArray * root = new JsonArray();
    root->addValue((JsonValue *)new JsonBoolean(true));
    JsonArray * prev = root;
    for (size_t i = 0; i < length; i++) {
        JsonArray * current = new JsonArray();
        current->addValue((JsonValue *)new JsonBoolean(true));

        prev->addValue(current);
        prev = current;
    }
    // std::string s = pretty(root->toJsonString());
    // loggerPrintf(LOGGER_TEST, "%s\n", s.c_str());
    // return s;
    return root->toJsonString();
}

static void parseObjectAndAssert(TestArg * t, std::string s, User expected, size_t expected_size) {
    try {
        ESharedPtr<JsonObject> obj_shared = dynamic_eshared_cast<JsonValue, JsonObject>(parse(s));
        JsonObject * obj = ESHAREDPTR_GET_PTR(obj_shared);
        if (obj->type == OBJECT) {
            User user(obj_shared);
            loggerPrintf(LOGGER_TEST_VERBOSE, "JSON To Parse: \n");
            loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", pretty(s).c_str());
            loggerPrintf(LOGGER_TEST_VERBOSE, "Parsed JSON - User Class: \n");
            loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", pretty(user.toJsonString()).c_str());
            loggerPrintf(LOGGER_TEST_VERBOSE, "Expected JSON - User Class: \n");
            loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", pretty(expected.toJsonString()).c_str());

            if (user != expected) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "User object mismatch.\n");
            }
            if (obj->keys.size() != expected_size) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "Unexpected number of keys.\n");
            }
            if (obj->values.size() != expected_size) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "Unexpected number of values.\n");
            }

            if (user == expected 
                && obj->keys.size() == expected_size 
                && obj->values.size() == expected_size) {
                t->fail = false;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
    }
}

static void parseObjectAndAssertStringComparison(TestArg * t, std::string s) {
    try {
        size_t i = 0;
        JsonValue * obj = ESHAREDPTR_GET_PTR(parse(s));

        std::string actual;
        if (obj->type == OBJECT) {
            // if (obj->toJsonString() == s 
            //     && obj->keys.size() == 77 * 2; 
            //     && obj->values.size() == 77 * 2) {

            // fprintf(obj->toJsonString() == s) {

            // lol.... nah.

            // TODO: write tests for the pretty function... because this relies on it.
            actual = pretty(dynamic_cast<JsonObject *>(obj)->toJsonString());
        } else if (obj->type == ARRAY) {
            actual = pretty(dynamic_cast<JsonArray *>(obj)->toJsonString());
        }

        // printf("size: %u", obj->keys.size());
        std::string expected = pretty(s.c_str());
        
        // printf("%s", actual.c_str());
        loggerPrintf(LOGGER_TEST_VERBOSE, "Expected\n%s, %ld\n", expected.c_str(), expected.size());
        loggerPrintf(LOGGER_TEST_VERBOSE, "Actual\n%s, %ld\n", actual.c_str(), actual.size());
        if (actual == expected) {
            t->fail = false;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
    }
}

static void parseObjectAndAssertMalformedJson(TestArg * t, std::string s) {
    try {
        size_t i = 0;
        ESharedPtr<JsonValue> obj = parse(s);
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
        t->fail = false;
    }
}

static void testJsonArray(TestArg * t);
static void testJsonNestedObject(TestArg * t);
static void testJsonNestedArray(TestArg * t);
static void testJsonEmptyObject(TestArg * t);
static void testJsonObjectWithName(TestArg * t);
static void testJsonObjectWithArray(TestArg * t);
// TODO: test for escaped-quoted string value..
static void testJsonAll(TestArg * t);

// negative test cases, these should all cause the parser to through an exception...
static void testJsonMalformedOpenObject(TestArg * t);
static void testJsonMalformedOpenArray(TestArg * t);
static void testJsonMalformedOpenNestedObject(TestArg * t);
static void testJsonMalformedOpenNestedArray(TestArg * t);
static void testJsonMalformedOpenEndKeyString(TestArg * t);
static void testJsonMalformedOpenEndValueString(TestArg * t);
static void testJsonMalformedOpenStartKeyString(TestArg * t);
static void testJsonMalformedOpenStartValueString(TestArg * t);
static void testJsonMalformedIncompleteBoolean(TestArg * t);
static void testJsonMalformedIncompleteNull(TestArg * t);
static void testJsonMalformedNonWhitespaceInBetweenTokens(TestArg * t);

static void testPretty(TestArg * t);

int main(int argc, char * argv[]) {
    Tester t("Json Parser Tests");

    t.addTest(testJsonNestedObject);
    t.addTest(testJsonNestedArray);
    t.addTest(testJsonArray);
    t.addTest(testJsonEmptyObject);
    t.addTest(testJsonObjectWithName);
    t.addTest(testJsonObjectWithArray);
    t.addTest(testJsonAll);

    t.addTest(testJsonMalformedOpenObject);
    t.addTest(testJsonMalformedOpenArray);
    t.addTest(testJsonMalformedOpenNestedObject);
    t.addTest(testJsonMalformedOpenNestedArray);
    t.addTest(testJsonMalformedOpenEndKeyString);
    t.addTest(testJsonMalformedOpenEndValueString);
    t.addTest(testJsonMalformedOpenStartKeyString);
    t.addTest(testJsonMalformedOpenStartValueString);
    t.addTest(testJsonMalformedIncompleteBoolean);
    t.addTest(testJsonMalformedIncompleteNull);
    t.addTest(testJsonMalformedNonWhitespaceInBetweenTokens);

    t.addTest(testPretty);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[1]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}

static void testJsonArray(TestArg * t) {
    std::string s("[false, true, false, false]");
    std::vector<bool> expected{false, true, false, false};
    try {
        size_t i = 0;
        JsonValue * obj = ESHAREDPTR_GET_PTR(parse(s));
        if (obj != nullptr) {
            if (obj->type == ARRAY) {
                loggerPrintf(LOGGER_TEST_VERBOSE, "JSON to Parse: \n");
                loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", pretty(s).c_str());
                JsonArray * values = dynamic_cast<JsonArray *>(obj);
                loggerPrintf(LOGGER_TEST_VERBOSE, "Parsed JSON Array: \n");
                loggerPrintf(LOGGER_TEST_VERBOSE, "%s\n", pretty(values->toJsonString()).c_str());
                size_t validation_count = 0;
                for (size_t i = 0; i < values->size(); i++) {
                    JsonValue * value = values->at(i);
                    if (value->type == BOOLEAN) {
                        bool actual = ((JsonBoolean *)value)->getValue();
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
    } catch (const std::exception& e) {
        std::cout << "Exception: \n" << e.what() << '\n';
    }
}

static void testJsonNestedObject(TestArg * t) {
    std::string s = createNestedObject(2);

    parseObjectAndAssertStringComparison(t, s);
}

static void testJsonNestedArray(TestArg * t) {
    std::string s = createNestedArray(2);

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

static void testJsonAll(TestArg * t) {
    std::string s("{ \n");
    s += "\"name\":\"username\", \n";
    s += "\"attributes\":\"attributes for user\", \n";
    s += "\"arr\": [false, true, false, false], \n";
    s += "\"dec\": 272727.1111, \n";
    // s += "\"nested\": { \"nested_name\": \"nested_value\" }, \n";
    s += "\"null_value\": null \n";
    s += "}\n";

    // Nested nested_obj;
    // nested_obj.name = "nested_value";

    User expected;
    expected.name = "username";
    expected.attributes = "attributes for user";
    std::vector<bool> expected_arr{false, true, false, false};
    expected.arr = expected_arr;
    expected.dec = 272727.1111;
    // expected.nested = nested_obj;

    // parseObjectAndAssert(t, s, expected, 6);
    // TODO: JsonArray destructor needs work... 
    parseObjectAndAssert(t, s, expected, 5);
}


static void testJsonMalformedOpenObject(TestArg * t) {
    std::string s("{");
    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedOpenArray(TestArg * t) {
    std::string s("[");
    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedOpenNestedObject(TestArg * t) {
    std::string s("{ \n");
    s += "\"nested\": {, \n";
    s += "}\n";
    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedOpenNestedArray(TestArg * t) {
    std::string s("[ \n");
    s += "[, \n";
    s += "]\n";
    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedOpenEndKeyString(TestArg * t) {
    std::string s("{ \n");
    s += "\"name: \"username\", \n";
    s += "}\n";

    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedOpenEndValueString(TestArg * t) {
    std::string s("{ \n");
    s += "\"name\": \"username, \n";
    s += "}\n";
    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedOpenStartKeyString(TestArg * t) {
    std::string s("{ \n");
    s += "name\": \"username\", \n";
    s += "}\n";
    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedOpenStartValueString(TestArg * t) {
    std::string s("{ \n");
    s += "\"name\": username\", \n";
    s += "}\n";
    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedIncompleteBoolean(TestArg * t) {
    std::string s("{ \n");
    s += "\"name\": fals, \n";
    s += "}\n";

    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedIncompleteNull(TestArg * t) {
    std::string s("{ \n");
    s += "\"name\": nu, \n";
    s += "}\n";

    parseObjectAndAssertMalformedJson(t, s);
}

static void testJsonMalformedNonWhitespaceInBetweenTokens(TestArg * t) {
    std::string s("{ \n");
    s += "\"name\":dnlanksal\"test\", \n";
    s += "}\n";

    parseObjectAndAssertMalformedJson(t, s);
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
    std::string actual = pretty(s);

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