#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include "reader.h"

#include "../c_lib/server.h"

// TODO: how to keep imports clean? don't want to rely on preprocessor stuff.?

#define HTTP_FIELD_MAX 64

using namespace WylesLibs;

namespace WylesLibs::Http {

static Array<std::string> FIELD_VALUES_TO_SPLIT{
    "connection",
	"sec-websocket-protocol",
	"accept-encoding"
};

static Array<std::string> FIELD_VALUES_TO_LOWER_CASE{
    "connection",
    "upgrade"
};

class HttpUrl {
    public:
        std::string domain;
        std::string path;
        std::string query_string;
        std::unordered_map<std::string, std::string> query_map;
}

class HttpRequest {
    public:
        std::string method;
        std::string path;
        std::string version;
        
        // hmm... lol... yeah.... 
        HttpUrl url;

        std::unordered_map<std::string, Array<std::string>> fields;
        Array<uint8_t> content;

        ~HttpRequest() {
            // free(r->method);
            // free(r->path);
            // free(r->version);
            // for (int i = 0; i < FIELD_MAX; i++) {
            //     free(r->fields[i].name);
            //     free(r->fields[i].value);
            // }
            // free(r->content);
        }

        void print() {
            printf("%s %s %s\n", this->method, this->path, this->version);

            int idx = 0;
            std::string field_name = this->fields[idx].name;
            while(field_name != "" && idx < FIELD_MAX) {
                // TODO: add field for cookies
                // TODO: add a separate field to request struct for this?
                printf("%s: %s\n", field_name, this->fields[idx].value);
                idx++;
                field_name = this->fields[idx].name;
            }

            if (this->content.size() != 0 && this->content_length > 0) {
                printf("---CONTENT_START(%d)---\n", this->content_length);
                for (int i = 0; i < this->content_length; i++) {
                    printf("%c", (char)this->content[i]);
                }
            }
            printf("---END\n");
        }
};

class HttpResponse {
    public:
        std::string method;
        std::string version;
        
        // // hmm... lol... yeah.... 
        // HttpUrl url;

        std::unordered_map<std::string, Array<std::string>> fields;
        Array<uint8_t> content;

        ~HttpResponse() {
            // free(r->method);
            // free(r->path);
            // free(r->version);
            // for (int i = 0; i < FIELD_MAX; i++) {
            //     free(r->fields[i].name);
            //     free(r->fields[i].value);
            // }
            // free(r->content);
        }

        void print() {
            printf("%s %s %s\n", this->method, this->path, this->version);

            int idx = 0;
            std::string field_name = this->fields[idx].name;
            while(field_name != "" && idx < FIELD_MAX) {
                // TODO: add field for cookies
                // TODO: add a separate field to request struct for this?
                printf("%s: %s\n", field_name, this->fields[idx].value);
                idx++;
                field_name = this->fields[idx].name;
            }

            if (this->content.size() != 0 && this->content_length > 0) {
                printf("---CONTENT_START(%d)---\n", this->content_length);
                for (int i = 0; i < this->content_length; i++) {
                    printf("%c", (char)this->content[i]);
                }
            }
            printf("---END\n");
        }
};

typedef void(RequestProcessor)(HttpRequest&);

void parseRequest(HttpRequest * request, IO:Reader * reader) {
    if (request == NULL || reader == NULL) {
        throw std::runtime_error("lol....");
    }
    request->method = reader->readUntil(' ').toString(); // TODO: error check... if NULL, then read error
    request->path = reader->readUntil(' ').toString();
    request->version = reader->readUntil('\n').toString();

    request->content_length = -1;
    int field_idx = 0; 
    int peek = reader->peekForEmptyLine();
    while (field_idx < FIELD_MAX && peek == 0) {
        std::string field_name = reader->readUntil(':', IO::byteOperationToLowerCaseAndIgnoreWhiteSpace).toString();
        // hmmm... need to trim too... lol
        //  is it ignore all whitespace or just that before and after first and last character? 
        
        //   so trim or ignore? 
        //   they are technically different? lol or rather ignore also implies trimming?

        // LMAO.... WOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
        ByteOperation byteOperation = IO::byteOperationIgnoreWhiteSpace;
        if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name)) {
            byteOperation = IO::byteOperationToLowerCaseAndIgnoreWhiteSpace;
        }
        if (FIELD_VALUES_TO_SPLIT.contains(field_name)) {
            char delimeter = 0x00;
            while (delimeter != '\n') {
                std::string value = reader->readUntil(",\n", byteOperation).toString();
                request->fields[field_name].push_back(value.substr(0, value.size()-1));
                delimeter = value.back();
            }
        } else {
            request->fields[field_name].push_back(reader->readUntil('\n', byteOperation).toString());
        }
        field_idx++;
        if (field_name == "Content-Length") {
            // TODO: negatives? use strtoul instead?
            request->content_length = atoi(value);
        }
        peek = reader->peekForEmptyLine();
    }
    if (peek == -1) {
        throw std::runtime_error("Reader Peek error.");
    }
    if (field_idx == FIELD_MAX) {
        throw std::runtime_error("Too many fields in request.");
    }
    if (request->content_length != -1) {
        // printf("request->content: %lx\n", request->content);
        request->content = reader->readBytes(request->content_length);
        // printf("request->content: %lx\n", request->content);
    }
}
class ConnectionUpgrader {
    public:
        std::string path;
        std::string protocol;

        ConnectionUpgrader(std::string path, std::string protocol): path(path), protocol(protocol) {}

        virtual uint8_t onConnection(int conn_fd) = 0;
}

class HttpConnectionUpgrader: public ConnectionUpgrader {
    public:
        HttpConnectionUpgrader(std::string path, std::string protocol): ConnectionUpgrader(path, protocol) {}

        uint8_t onConnection(int conn_fd) {
 
        }
}

// voila!
class HttpConnection {
    private:
        RequestProcessor processor;
        Array<ConnectionUpgrader> upgraders;
    public:
        HttpConnection(RequestProcessor processor, Array<ConnectionUpgrader> upgraders): processor(processor), upgraders(upgraders) {}
        uint8_t onConnection(int conn_fd) {
            HttpRequest * request = new HttpRequest();
            IO:Reader * reader = new IO:Reader(conn_fd);
            try {
                parseRequest(request, reader);
                request.print();

                if (request.fields["upgrade"].contains("websocket") && request.fields["connection"].contains("upgrade")) {
            		Array<std::string> protocols = request.fields["sec-websocket-protocol"];
            		for (size_t i = 0; i < protocols.size(); i++) {
                        for (size_t x = 0; i < upgraders.size(); i++) {
                            std::string protocol = protocols[i];
							if upgrader.path == request.url.path && protocol == upgrader.protocol {
								response.StatusCode = 101
								response.fields["Upgrade"] = "websocket"
								response.fields["Connection"] = "upgrade"
								response.fields["Sec-Websocket-Protocol"] = protocol
								response.fields["Sec-WebSocket-Version"] = "13"
								response.fields["Sec-WebSocket-Extensions"] = ""
 
								magic_key := response.Fields["sec-websocket-key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
								hash := sha1.New()
								hash.Write([]byte(magic_key))
								checksum := hash.Sum(nil)
								dst := make([]byte, base64.StdEncoding.EncodedLen(len(checksum)))
								base64.StdEncoding.Encode(dst, checksum)
								response.Fields["Sec-WebSocket-Accept"] = string(dst)
 
								if err := utils.WriteBytes(c, response.Bytes()); err != nil {
									panic(err)
								}

								upgrader.onConnection(conn_fd);
 
								// NOTE: the upgrade function should indefinetly block until the connection is intended to be closed.
								return
                            }
                        }
                    }
                }

                delete reader;
                this->processor(request);
            } catch (const std::exception& e) {
                // Again, low overhead.... so, should be fine...
                std::cout << "Exception: \n" << e.what() << '\n';
            }
  
            delete request;
            delete reader;
  
            close(conn_fd); // doc's say you shouldn't retry close so ignore ret
 
            return 1;
        }
};

class HttpServerConfig: Json::JsonBase {
    public:
        std::string static_path;
        std::string root_html_file;
        std::string address;
        std::string port;

        HttpServerConfig() {}
        HttpServerConfig(std::string filepath): HttpServerConfig((Json::JsonObject *)Json::parse(File::read(filepath).toString())) {}
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
            return !this == other;
        }
}

};
