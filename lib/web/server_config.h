#ifndef WYLESLIBS_SERVER_CONFIG_H
#define WYLESLIBS_SERVER_CONFIG_H

#include "string_format.h"
#include "parser/json/json.h"
#include "file/stream_factory.h"

#include <memory>
#include "memory/pointers.h"

using namespace WylesLibs;
using namespace WylesLibs::Parser::Json;
using namespace WylesLibs::File;

namespace WylesLibs {

class ServerConfig: public StrictJsonObject {
    protected:
        std::string toJsonElements() override {
            std::string s("\"resources_root\": ");
            s += JsonString(this->resources_root).toJsonString();
            return s;
        }

    public:
        std::string filepath;
        std::string resources_root;

        ServerConfig(): resources_root("./") {}
        ServerConfig(std::string filepath): ServerConfig(
            parseFile(ESharedPtr<StreamFactory>(new StreamFactory), filepath), filepath
        ) {}
        ServerConfig(ESharedPtr<JsonValue> obj_shared, std::string fp) {
            filepath = fp;
            JsonObject * obj = dynamic_cast<JsonObject *>(ESHAREDPTR_GET_PTR(obj_shared));
            bool resources_root_required = true;
            for (auto e: *obj) {
                std::string key = e.first;
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = e.second;
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