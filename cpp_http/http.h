#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include <filesystem>

#include "reader/reader.h"
#include "server.h"
#include "config.h"
#include "connection.h"

#include "file.h"

// lol, yeah think about this... there aren't that many items...
#include <unordered_map>
// TODO: how to keep imports clean? don't want to rely on preprocessor stuff.?

#define HTTP_FIELD_MAX 64

using namespace WylesLibs;

namespace WylesLibs::Http {

// static Array<std::string> FIELD_VALUES_TO_LOWER_CASE{
//     "connection",
//     "upgrade"
// };

class Url {
    public:
        std::string path;
        std::unordered_map<std::string, std::string> query_map;
};

class HttpRequest {
    public:
        std::string method;
        Url url;
        std::string version;
        
        std::unordered_map<std::string, Array<std::string>> fields;
        std::unordered_map<std::string, Array<std::string>> cookies;
        // TODO:
        // if content-type == then different types?
        //  JsonObject content;
        //  MultipartFile content;
        //  then use reader to parse json?

        //  update json parser to support both string and reader?
        Array<uint8_t> content;
        size_t content_length;

        void print() {
            // printf("%s %s %s\n", this->method, this->path, this->version);

            // int idx = 0;
            // std::string field_name = this->fields[idx].name;
            // while(field_name != "" && idx < FIELD_MAX) {
            //     // TODO: add field for cookies
            //     // TODO: add a separate field to request struct for this?
            //     printf("%s: %s\n", field_name, this->fields[idx].value);
            //     idx++;
            //     field_name = this->fields[idx].name;
            // }

            // if (this->content.size() != 0 && this->content_length > 0) {
            //     printf("---CONTENT_START(%d)---\n", this->content_length);
            //     for (int i = 0; i < this->content_length; i++) {
            //         printf("%c", (char)this->content[i]);
            //     }
            // }
            // printf("---END\n");
        }
};

class HttpResponse {
    public:
        std::string status_code;
        std::string method;
        std::string version;
        std::unordered_map<std::string, std::string> fields;
        std::unordered_map<std::string, std::string> cookies;
        Array<uint8_t> content;

        HttpResponse(): version("HTTP/1.1"), status_code("500") {
			fields["Connection"].append("close");
        }

        std::string toString() {
            // header = this->version + " " + strconv.Itoa(status_code) + " " + status_map[r.StatusCode] + "\n";
            std::string response = this->version + " " + this->status_code + "\n";
            for (const auto& [key, value] : this->fields) {
				response += key + ": " + value + "\n";
			}
            for (const auto& [key, value] : this->cookies) {
				response += "set-cookie: " + value + "\n";
			}
			response += "\n";
            response += content.toString();
            return response;
        }
};

typedef HttpResponse(* RequestProcessor)(HttpRequest *);
// voila!
class HttpConnection {
    private:
        RequestProcessor processor;
        Array<ConnectionUpgrader *> upgraders;
        HttpServerConfig config;
        std::unordered_map<std::string, std::string> static_paths; 

        void parseRequest(HttpRequest * request, Reader * reader);
        bool handleWebsocketRequest(int conn_fd, HttpRequest * request);
        bool handleStaticRequest(int conn_fd, HttpRequest * request);
    public:
        HttpConnection() {}
        HttpConnection(HttpServerConfig config, RequestProcessor processor, Array<ConnectionUpgrader *> upgraders): 
            config(config), processor(processor), upgraders(upgraders) {
                for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(config.static_path)) {
                    std::string path = "/" + dir_entry.path().string();
                    std::string ext = dir_entry.path().extension().string();
                    printf("path: %s\n", path.c_str());
                    printf("ext: %s\n", ext.c_str());
					if (ext == "html") {
						static_paths[path] = "text/html";
					} else if (ext == "js") {
						static_paths[path] = "text/javascript";
					} else if (ext == "css") {
						static_paths[path] = "text/css";
                    } else {
						static_paths[path] = "text/plain";
                    }
					// if (ext == ".html") {
					// 	static_paths[path] = "text/html"
					// } else if (ext == ".js") {
					// 	static_paths[path] = "text/javascript"
					// } else if (ext == ".css") {
					// 	static_paths[path] = "text/css"
                    // } else {
					// 	static_paths[path] = "none"
                    // }
                }
        }

        uint8_t onConnection(int conn_fd);
};

};
