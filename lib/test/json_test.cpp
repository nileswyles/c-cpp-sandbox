#include "json.h"

using namespace WylesLibs::Json;

// lol
void printProcessFunc(std::string key, WylesLibs::Json::JsonValue * value) {
    WylesLibs::Json::JsonType type = value->type;
    loggerPrintf(LOGGER_TEST, "value type: %d\n", type);

    if (type == WylesLibs::Json::BOOLEAN) {
        WylesLibs::Json::JsonBoolean * booleanValue = (JsonBoolean *)value;
        loggerPrintf(LOGGER_TEST, "boolean value: %u\n", booleanValue->boolean);
    } else if (type == WylesLibs::Json::STRING) {
        // when created, sizeof(JsonString) on stack? or sizeof(JsonValue)? lol 
        loggerPrintf(LOGGER_TEST, "lol?\n");
        WylesLibs::Json::JsonString * stringValue = (JsonString *)value;
        loggerPrintf(LOGGER_TEST, "string value: %s\n", stringValue->s.c_str());
    } else if (type == WylesLibs::Json::NUMBER) {
        // when created, sizeof(JsonString) on stack? or sizeof(JsonValue)? lol 
        loggerPrintf(LOGGER_TEST, "lol....\n");
        WylesLibs::Json::JsonNumber * numberValue = (JsonNumber *)value;
        loggerPrintf(LOGGER_TEST, "number value: %f\n", numberValue->number);

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

    JsonObject obj = parse(&s);

    loggerPrintf(LOGGER_TEST, "Num Keys: %lu\n", obj.keys.getSize());
    loggerPrintf(LOGGER_TEST, "Num Values: %lu\n", obj.values.getSize());
    for (size_t i = 0; i < obj.keys.getSize(); i++) {
        loggerPrintf(LOGGER_TEST, "key: %s\n", obj.keys.buf[i].c_str());

        // lol?
        WylesLibs::Json::JsonValue * value = obj.values.buf[i];
        // TODO:
        // this means every class has to explicitly delete values...
        //  LAMEEEEEEEEEEEEEEEEEEEEEEEEEEEEE, I was hoping lmao

        // better?
        processValue(obj.keys.buf[i], value, printProcessFunc);
    }

    // if already freed, then do nothing? lol is that not how free works?
    loggerPrintf(LOGGER_TEST, "...\n");
    return 0;
}