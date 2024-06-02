#ifndef WYLESLIBS_HTTP_SERVER_CONFIG_H
#define WYLESLIBS_HTTP_SERVER_CONFIG_H

#include "json/json.h"
#include "file.h"

using namespace WylesLibs;

namespace WylesLibs::Http {

class HttpServerConfig: Json::JsonBase {
    public:
        std::string static_path;
        std::string root_html_file;
        std::string address;
        std::string port;

        HttpServerConfig() {}
        // file sig should support strings... 
        HttpServerConfig(std::string filepath): 
            HttpServerConfig((Json::JsonObject *)Json::parse(File::read(filepath).toString())) {}
        HttpServerConfig(Json::JsonObject * obj) {
            size_t validation_count = 0;
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                Json::JsonValue * value = obj->values.at(i);
                if (key == "static_path") {
                    static_path = Json::setVariableFromJsonValue<std::string>(value, validation_count);
                } else if (key == "address") {
                    address = Json::setVariableFromJsonValue<std::string>(value, validation_count);
                } else if (key == "port") {
                    port = Json::setVariableFromJsonValue<std::string>(value, validation_count);
                } else if (key == "root_html_file") {
                    root_html_file = Json::setVariableFromJsonValue<std::string>(value, validation_count);
                }
            }
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "validation count: %lu\n", validation_count);
            if (validation_count != obj->keys.size()) {
                throw std::runtime_error("Failed to create HttpServerConfig from json object.");
            }
        }

        std::string toJsonString() {
            std::string s("{");
            s += "\"static_path\": ";
            s += Json::JsonString(this->static_path).toJsonString();
            s += ",";

            s += "\"address\": ";
            s += Json::JsonString(this->address).toJsonString();
            s += ",";

            s += "\"port\": ";
            s += Json::JsonString(this->port).toJsonString();
            s += ",";

            s += "\"root_html_file\": ";
            s += Json::JsonString(this->root_html_file).toJsonString();
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