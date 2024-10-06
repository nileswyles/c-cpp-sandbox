#ifndef WYLESLIBS_JSON_OBJECT_H
#define WYLESLIBS_JSON_OBJECT_H

#include "json_parser.h"
#include "json_array.h"

#include "logger.h"

#include <vector>
#include <string>

namespace WylesLibs::Parser::Json {
class JsonObject: public JsonValue {
    public:
        std::vector<std::string> keys; 
        JsonArray values;
        size_t depth;
        JsonObject(): JsonObject(0) {}
        JsonObject(size_t depth): depth(depth), JsonValue(OBJECT) {
            if (depth > MAX_JSON_DEPTH) {
                throw std::runtime_error("JsonObject creation error... TOO MUCH DEPTH!");
            }
            values.depth = depth;
        }
        ~JsonObject() override = default;

        void addKey(std::string key) {
            this->keys.push_back(key);
            loggerPrintf(LOGGER_DEBUG, "Added json key! @ %s\n", key.c_str());
        }

        std::string toJsonString() final override;
};

}
#endif