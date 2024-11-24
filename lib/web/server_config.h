#ifndef WYLESLIBS_SERVER_CONFIG_H
#define WYLESLIBS_SERVER_CONFIG_H

#include "string_format.h"
#include "parser/json/json.h"
#include "file/stream_factory.h"

#include <memory>
#include "eshared_ptr.h"

using namespace WylesLibs;
using namespace WylesLibs::Parser::Json;
using namespace WylesLibs::File;

namespace WylesLibs {

class ServerConfig: public JsonBase {
    public:
        std::string filepath;
        std::string resources_root;

        ServerConfig(): resources_root("./") {}
        ServerConfig(std::string filepath): ServerConfig(
            parseFile(ESharedPtr<StreamFactory>(std::make_shared<StreamFactory>()), filepath)
        ) {}
        ServerConfig(ESharedPtr<JsonValue> obj_shared) {
            JsonObject * obj = dynamic_cast<JsonObject *>(obj_shared.getPtr(__func__));
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            bool resources_root_required = true;
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = obj->values.at(i);
                if (key == "resources_root") {
                    resources_root = setVariableFromJsonValue<std::string>(value);
                    resources_root_required = false;
                }
            }
            if (true == resources_root_required) {
                std::string msg = format("The 'resources_root' field is missing... Check the configuration file at path: {}", filepath);
                loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
        }
        ~ServerConfig() override = default;

        std::string toJsonElements() {
            std::string s("\"resources_root\": ");
            s += JsonString(this->resources_root).toJsonString();
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