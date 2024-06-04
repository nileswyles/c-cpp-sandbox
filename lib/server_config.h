#ifndef WYLESLIBS_HTTP_SERVER_CONFIG_H
#define WYLESLIBS_HTTP_SERVER_CONFIG_H

#include "parser/json/json.h"
#include "file.h"

using namespace WylesLibs::Parser::Json;

namespace WylesLibs {

class ServerConfig: public JsonBase {
    public:
        std::string resources_root;
        std::string db_path;

        ServerConfig() = default;
        ServerConfig(std::string filepath): 
            ServerConfig((JsonObject *)parseFile(filepath)) {}
        ServerConfig(JsonObject * obj) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = obj->values.at(i);
                if (key == "resources_root") {
                    resources_root = setVariableFromJsonValue<std::string>(value);
                }
            }
        }

        std::string toJsonString() {
            std::string s("{");
            s += "\"resources_root\": ";
            s += JsonString(this->resources_root).toJsonString();
            s += "}";

            return s;
        }

        bool operator == (const ServerConfig& other) {
            if(this->resources_root != other.resources_root) {
                return false;
            }
            return true;
        }

        bool operator != (const ServerConfig& other) {
            return !(*this == other);
        }
};

}
#endif 