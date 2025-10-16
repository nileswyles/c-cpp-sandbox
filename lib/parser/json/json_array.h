#ifndef WYLESLIBS_JSON_ARRAY_H
#define WYLESLIBS_JSON_ARRAY_H

#include "json_node.h"
#include "json_mix.h"
#include "jstring.h"

#include <vector>
#include <string>

#ifndef LOGGER_JSON_ARRAY
#define LOGGER_JSON_ARRAY 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_ARRAY
#include "logger.h"

namespace WylesLibs::Parser::Json {
    // TODO: confirm what public keyword means in this context.
    class JsonArray: public JsonValue, public std::vector<JsonValue *> {
        public:
            size_t depth;
            JsonArray(): JsonArray(0) {}
            JsonArray(size_t depth): depth(depth), JsonValue(WylesLibs::Parser::Json::ARRAY), std::vector<JsonValue *>() {
                if (depth > MAX_JSON_DEPTH) {
                    throw std::runtime_error("JsonArray: creation error... TOO MUCH DEPTH!");
                }
            }

            // ! IMPORTANT - This JsonArray type is also used to represent the JsonObject values.
            //      If that needs to change, remember to delete JsonValue pointers.
            //      Polymorphism enables better syntax for working directly with Json-types.
            //      But the real reason for this is because I think it's easier to conceptualize using OOP concepts.
            //      (biased by Java background?), ironically - structure helps here - but it's not always the case. JS is also awesome. Represent! BARS

            //      hmm... yeah, again generalization for when to generalize?
            //      I think generally the context will tell you, but verify?
            //      If operating on multiple types, weak types (in other words, easy conversion) is preferred?

            //      And you can make the distinction between language-defined semantics (global rule) and program-defined semantics.
            //      But it's really all just program-defined. MIND BLOWN!

            //      lol... and with some reflection - like in Java, you can skip this intermediate stuff? whether you should is another question?

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

            void compile(std::vector<JsonNode> node);
            void addValue(JsonValue * value) {
                this->push_back(value);
                loggerPrintf(LOGGER_DEBUG, "Added json value object! @ %p\n", value);
            }
            void remove(size_t i);
            std::string toJsonString() final override;
            // TODO: hmm... if operator protected? but that's what public std::vector is for? yeah, this might not be necessary.
            //      that said, mixed behaviour in practice.
            //JsonValue * operator= (size_t i) {
            //    return (*this)[i];
            //}
            std::vector<JsonValue *>& operator* () {
                return *this;
            }
    };
}
#endif