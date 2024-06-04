#ifndef WYLESLIBS_HTTP_SERVER_CONFIG_H
#define WYLESLIBS_HTTP_SERVER_CONFIG_H

#include "parser/json/json.h"
#include "server_config.h"
#include "file.h"

using namespace WylesLibs;
using namespace WylesLibs::Parser::Json;

namespace WylesLibs::Http {

class HttpServerConfig: public ServerConfig {
    public:
        std::string static_path;
        std::string root_html_file;
        std::string address;
        std::string port;

        HttpServerConfig() = default;
        HttpServerConfig(std::string filepath): HttpServerConfig((JsonObject *)parseFile(filepath)) {}
        HttpServerConfig(JsonObject * obj): ServerConfig(obj) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = obj->values.at(i);
                if (key == "static_path") {
                    static_path = setVariableFromJsonValue<std::string>(value);
                } else if (key == "address") {
                    address = setVariableFromJsonValue<std::string>(value);
                } else if (key == "port") {
                    port = setVariableFromJsonValue<std::string>(value);
                } else if (key == "root_html_file") {
                    root_html_file = setVariableFromJsonValue<std::string>(value);
                }
            }
        }

        std::string toJsonString() {
            std::string s("{");
            s += "\"static_path\": ";
            s += JsonString(this->static_path).toJsonString();
            s += ",";

            s += "\"address\": ";
            s += JsonString(this->address).toJsonString();
            s += ",";

            s += "\"port\": ";
            s += JsonString(this->port).toJsonString();
            s += ",";

            s += "\"root_html_file\": ";
            s += JsonString(this->root_html_file).toJsonString();
            s += "}";

            return s;
        }

        bool operator == (const HttpServerConfig& other) {
            if(this->static_path != other.static_path) {
                return false;
            }
            if(this->address != other.address) {
                return false;
            }
            if(this->port != other.port) {
                return false;
            }
            if(this->root_html_file != other.root_html_file) {
                return false;
            }
            return true;
        }

        bool operator != (const HttpServerConfig& other) {
            return !(*this == other);
        }
};

}
#endif 