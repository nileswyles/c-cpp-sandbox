#ifndef WYLESLIBS_JSON_OBJECT_STRICT_H
#define WYLESLIBS_JSON_OBJECT_STRICT_H

#include <string>

#include "file/file.h"
#include "parser/json/json_mix.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

namespace WylesLibs::Parser::Json {

// base class for "JsonObject->CppType" map classes.
// This facilates an organized pattern of working with Json, when using the JsonObject directly becomes to cumbersome.
class StrictJsonObject: public JsonBase {
    protected:
        // Provides a string containing the json object's elements.
        virtual std::string toJsonElements() {
            return "";
        }
    public:
        std::string filepath;
        size_t depth;
        StrictJsonObject() = default;
        StrictJsonObject(size_t p, std::string fp): depth(p), filepath(fp) {}
        ~StrictJsonObject() override = default;

        void sync() {
            static FileManager file_manager;
            file_manager.write<std::string>(this->filepath, this->toJsonString(), false);
        }

        // Provides a string containing the json object.
        std::string toJsonString() override {
            std::string s("{\n");
            s += this->toJsonElements();
            s += "}\n";
            return s;
        }
};

}
#endif