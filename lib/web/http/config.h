#ifndef WYLESLIBS_HTTP_SERVER_CONFIG_H
#define WYLESLIBS_HTTP_SERVER_CONFIG_H

#include "parser/json/json.h"
#include "web/server_config.h"
#include "file.h"

#ifndef LOGGER_HTTP_SERVER_CONFIG
#define LOGGER_HTTP_SERVER_CONFIG 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP_SERVER_CONFIG
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Parser::Json;

namespace WylesLibs::Http {

class HttpServerConfig: public ServerConfig {
    public:
        std::string static_path;
        std::string root_html_file;
        std::string address;
        uint16_t port;

        HttpServerConfig(): static_path("./"), root_html_file("index.html"), address("127.0.0.1"), port(8080) {}
        HttpServerConfig(std::string filepath): HttpServerConfig((JsonObject *)parseFile(filepath)) {}
        HttpServerConfig(JsonObject * obj): ServerConfig(obj) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = obj->values.at(i);
                // TODO: validation... like root_html_file should be defined and size>0
                if (key == "static_path") {
                    static_path = setVariableFromJsonValue<std::string>(value);
                } else if (key == "address") {
                    address = setVariableFromJsonValue<std::string>(value);
                } else if (key == "port") {
                    port = (uint16_t)setVariableFromJsonValue<double>(value);
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
            s += JsonNumber(this->port, 4, 0).toJsonString();
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