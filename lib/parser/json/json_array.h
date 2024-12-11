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

namespace WylesLibs::Parser::Json {
class JsonArray: public JsonValue, public std::vector<JsonValue *> {
    public:
        size_t depth;
        JsonArray(): JsonArray(0) {}
        JsonArray(size_t depth): depth(depth), JsonValue(WylesLibs::Parser::Json::ARRAY), std::vector<JsonValue *>() {
            if (depth > MAX_JSON_DEPTH) {
                throw std::runtime_error("JsonArray: creation error... TOO MUCH DEPTH!");
            }
        }
        // ~JsonArray() override = default;
        // ! IMPORTANT - 
        //  given the seg fault when deleting the (OBJECT) JsonValues using the custom destructor, 
        //      does this mean the default destructor didn't actually call delete on the pointers it held? 
        //      Obviously? And it's why the below is required?
        ~JsonArray() override {
            // TODO. let cpp do it's magic... and don't use pointers here? or use shared_ptrs?.... 
            //  Is that okay? think about limits... and how they actually work. 
            size_t size = this->size();
            for (size_t i = 0; i < size; i++) {
                JsonValue * value = (*this)[i];
                if (value != nullptr) {
                    loggerPrintf(LOGGER_DEBUG, "Making sure to free pointer! @ %p, '%s'\n", value, value->toJsonString().c_str());
                    delete value;
                }
            }
        }

        void addValue(JsonValue * value) {
            this->push_back(value);
            loggerPrintf(LOGGER_DEBUG, "Added json value object! @ %p\n", value);
        }

        std::string toJsonString() final override;
};
}

#endif