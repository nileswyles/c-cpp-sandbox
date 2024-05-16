#ifndef WYLESLIBS_JSON_ARRAY_H
#define WYLESLIBS_JSON_ARRAY_H

#include "json_parser.h"

#include "logger.h"

#include <vector>
#include <string>

namespace WylesLibs::Json {
class JsonArray: public JsonValue, public std::vector<JsonValue *> {
    public:
        JsonArray(): JsonValue(ARRAY), std::vector<JsonValue *>() {}
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