#ifndef WYLESLIBS_HTTP_SERVER_CONFIG_H
#define WYLESLIBS_HTTP_SERVER_CONFIG_H

#include "parser/json/json.h"
#include "web/server_config.h"
#include "file/stream_factory.h"

#include <memory>
#include "memory/pointers.h"
#include <set>
#include <string>

#ifndef LOGGER_HTTP_SERVER_CONFIG
#define LOGGER_HTTP_SERVER_CONFIG 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP_SERVER_CONFIG
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Parser::Json;
using namespace WylesLibs::File;

namespace WylesLibs::Http {
class HttpServerConfig: public ServerConfig {
    public:
        std::string static_path;
        std::string root_html_file;
        std::string address;
        uint16_t port;
        bool tls_enabled;
        std::string path_to_trust_chain_cert;
        std::string path_to_cert; // pem to include private key?, or p7/p12 file?
        std::string path_to_private_key;
        bool client_auth_enabled;

        HttpServerConfig(): static_path("./"), root_html_file("index.html"), address("127.0.0.1"), port(8080) {}
        HttpServerConfig(std::string filepath): HttpServerConfig(
            parseFile(ESharedPtr<StreamFactory>(new StreamFactory), filepath)
        ) {}
        HttpServerConfig(ESharedPtr<JsonValue> obj_shared): ServerConfig(obj_shared) {
            JsonObject * obj = dynamic_cast<JsonObject *>(ESHAREDPTR_GET_PTR(obj_shared));
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            // Defaults for optional fields...
            // string defaults are already "" but let's be explicit....
            tls_enabled = false;
            path_to_trust_chain_cert = "";
            path_to_cert = "";
            path_to_private_key = "";
            client_auth_enabled = false;

            std::set<std::string> required_fields{
                "static_path",
                "address",
                "port",
                "address",
                "root_html_file"
            };
            // TODO: Implement better verification? Maybe some class to better support "required if field is enabled". Not strictly necessary now because we check that SSL is enabled properly.
            std::set<std::string> tls_required_fields{
                "path_to_trust_chain_cert",
                "path_to_cert",
                "path_to_private_key"
            };
            std::string key;
            for (size_t i = 0; i < obj->keys.size(); i++) {
                key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = obj->values.at(i);
                if (key == "static_path") {
                    static_path = setVariableFromJsonValue<std::string>(value);
                    required_fields.erase("static_path");
                } else if (key == "address") {
                    address = setVariableFromJsonValue<std::string>(value);
                    required_fields.erase("address");
                } else if (key == "port") {
                    port = (uint16_t)setVariableFromJsonValue<double>(value);
                    required_fields.erase("port");
                } else if (key == "root_html_file") {
                    root_html_file = setVariableFromJsonValue<std::string>(value);
                    required_fields.erase("root_html_file");
                } else if (key == "tls_enabled") {
                    tls_enabled = setVariableFromJsonValue<bool>(value);
                } else if (key == "path_to_trust_chain_cert") {
                    path_to_trust_chain_cert = setVariableFromJsonValue<std::string>(value);
                    tls_required_fields.erase("path_to_trust_chain_cert");
                } else if (key == "path_to_cert") {
                    path_to_cert = setVariableFromJsonValue<std::string>(value);
                    tls_required_fields.erase("path_to_cert");
                } else if (key == "path_to_private_key") {
                    path_to_private_key = setVariableFromJsonValue<std::string>(value);
                    tls_required_fields.erase("path_to_private_key");
                } else if (key == "client_auth_enabled") {
                    client_auth_enabled = setVariableFromJsonValue<bool>(value);
                }
            }

            if (required_fields.size() > 0 || (true == tls_enabled && tls_required_fields.size() > 0)) {
                std::string msg = format("The configuration file is missing required fields... Check the configuration file at path: {}", filepath);
                msg += ".\n The missing fields are listed below: ";
                for (auto field: required_fields) {
                    msg += '\n';
                    msg += field;
                }
                if (tls_enabled) {
                    for (auto field: tls_required_fields) {
                        msg += '\n';
                        msg += field;
                    }
                }
                loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
        }
        ~HttpServerConfig() override = default;

        std::string toJsonElements() {
            std::string s("\"static_path\": ");
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
            s += ",";
            
            s += "\"tls_enabled\": ";
            s += JsonBoolean(this->tls_enabled).toJsonString();
            s += ",";

            s += "\"path_to_trust_chain_cert\": ";
            s += JsonString(this->path_to_trust_chain_cert).toJsonString();
            s += ",";

            s += "\"path_to_cert\": ";
            s += JsonString(this->path_to_cert).toJsonString();
            s += ",";

            s += "\"path_to_private_key\": ";
            s += JsonString(this->path_to_private_key).toJsonString();
            s += ",";

            s += "\"client_auth_enabled\": ";
            s += JsonBoolean(this->client_auth_enabled).toJsonString();
            return s;

        }

        bool operator ==(const HttpServerConfig& other) {
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
            if(this->tls_enabled != other.tls_enabled) {
                return false;
            }
            if(this->path_to_cert != other.path_to_cert) {
                return false;
            }
            if(this->path_to_private_key != other.path_to_private_key) {
                return false;
            }
            return true;
        }

        bool operator !=(const HttpServerConfig& other) {
            return !(*this == other);
        }
};

}
#endif 