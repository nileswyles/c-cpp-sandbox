#include "json.h"

using namespace WylesLibs::Json;

int main() {

    const char * s = "{\"test\":false, \"test2\":\"value\"}";

    loggerPrintf(LOGGER_TEST, "JSON STRING: %s\n", s);

    JsonObject obj = parse(s);

    WylesLibs::Array<std::string> arr = obj.keys;
    loggerPrintf(LOGGER_TEST, "Num Keys: %lu\n", arr.getSize());
    for (size_t i = 0; i < arr.getSize(); i++) {
        loggerPrintf(LOGGER_TEST, "key: %s\n", arr.buf[i].c_str());
        // loggerPrintf(LOGGER_TEST, "value: %s\n", obj.values[i].c_str());
    }

    return 0;
}