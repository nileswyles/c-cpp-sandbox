#ifndef WYLESLIBS_JSON_ARRAY_H
#define WYLESLIBS_JSON_ARRAY_H

#include "json_parser.h"

#include <vector>
#include <string>

#ifndef LOGGER_JSON_ARRAY
#define LOGGER_JSON_ARRAY 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_ARRAY
#include "logger.h"

namespace WylesLibs::Json {
class JsonArray: public JsonValue, public std::vector<JsonValue *> {
    public:
        size_t depth;
        JsonType value_type;
        // TODO: 
        //  down side of this is no compiler check? so maybe that's why required to define default constructor?
        JsonArray(): JsonArray(0) {}
        JsonArray(size_t depth): depth(depth), JsonValue(ARRAY), std::vector<JsonValue *>() {
            if (depth > MAX_JSON_DEPTH) {
                throw std::runtime_error("JsonArray: creation error... TOO MUCH DEPTH!");
            }
        }
        ~JsonArray() {
            // TODO. let cpp do it's magic... and don't use pointers here?.... 
            //  Is that okay? think about limits... and how they actually work. 
            for (size_t i = 0; i < this->size(); i++) {
                loggerPrintf(LOGGER_DEBUG, "Making sure to free pointer! @ %p\n", this->at(i));
                // because this->[] isn't valid :)
                delete this->at(i);
            }
        }

        void addValue(JsonValue * value) {
            this->push_back(value);
            loggerPrintf(LOGGER_DEBUG, "Added json value object! @ %p\n", value);
        }

        std::string toJsonString();
};

}

#endif