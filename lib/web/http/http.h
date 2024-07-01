#ifndef WYLESLIB_HTTP_H
#define WYLESLIB_HTTP_H

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
#include "web/server.h"
#include "config.h"
#include "connection.h"
#include "web/authorization.h"

#include "parser/multipart/parse_formdata.h"
#include "parser/multipart/multipart_file.h"

#include "file.h"

#include <unordered_map>
#include <map>

#define HTTP_FIELD_MAX 64

using namespace WylesLibs;
using namespace WylesLibs::Parser::Multipart;

namespace WylesLibs::Http {

static Array<const char *> FIELD_VALUES_TO_LOWER_CASE{
    "connection",
    "upgrade"
};

class Url {
    public:
        std::string path;
        std::unordered_map<std::string, std::string> query_map;
        
        Url() {}
};

class HttpRequest {
    public:
        std::string method;
        Url url;
        std::string version;
        
        std::map<std::string, Array<std::string>> fields;
        std::map<std::string, Array<std::string>> cookies;

        size_t content_length;
        JsonValue * json_content;
        Array<MultipartFile> files;
        std::unordered_map<std::string, std::string> form_content;
        Array<uint8_t> content;

        Authorization auth;

        HttpRequest(): json_content(nullptr) {}
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

// TODO: func ptr types... 
// typedef HttpResponse *(RequestProcessor)(HttpRequest *);
//  vs.
// typedef HttpResponse *(* RequestProcessor)(HttpRequest *);
typedef HttpResponse *(RequestProcessor)(HttpRequest *);
typedef void(* RequestFilter)(HttpRequest *);
typedef void(* ResponseFilter)(HttpResponse *);

// voila!
class HttpConnection {
    private:
        RequestProcessor * processor;
        map<std::string, map<std::string, RequestProcessor *>> request_map;
        Array<RequestFilter> request_filters;
        Array<ResponseFilter> response_filters;
        Array<ConnectionUpgrader *> upgraders;
        HttpServerConfig config;
        std::unordered_map<std::string, std::string> static_paths; 

        void parseRequest(HttpRequest * request, Reader * reader);
        bool handleWebsocketRequest(int conn_fd, HttpRequest * request);
        bool handleStaticRequest(int conn_fd, HttpRequest * request);
        bool handleTimeoutRequests(int conn_fd, HttpRequest * request);

        HttpResponse * requestDispatcher(HttpRequest * request);
        // hmm... private static member?
        static void initializeStaticPaths(HttpServerConfig config, std::unordered_map<std::string, std::string> * static_paths) {
            loggerPrintf(LOGGER_DEBUG, "Static Paths: %s\n", config.static_path.c_str());
                for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(config.static_path)) {
                    std::string path = dir_entry.path().string();
                    std::string ext = dir_entry.path().extension().string();
					if (ext == ".html") {
						(*static_paths)[path] = "text/html";
					} else if (ext == ".js") {
						(*static_paths)[path] = "text/javascript";
					} else if (ext == ".css") {
						(*static_paths)[path] = "text/css";
                    } else {
						(*static_paths)[path] = "none";
                    }
                }
        }
    public:
        HttpConnection() {}
        // haha, funny how that worked out...
        HttpConnection(HttpServerConfig pConfig, map<std::string, map<std::string, RequestProcessor *>> pRequest_map, 
                        Array<RequestFilter> pRequest_filters, Array<ResponseFilter> pResponse_filters, 
                        Array<ConnectionUpgrader *> pUpgraders) {
            config = pConfig;
            request_map = pRequest_map;
            request_filters = pRequest_filters;
            response_filters = pResponse_filters;
            upgraders = pUpgraders;
            processor = nullptr;
            HttpConnection::initializeStaticPaths(config, &static_paths);
        }
        HttpConnection(HttpServerConfig config, RequestProcessor * processor, Array<ConnectionUpgrader *> upgraders): 
            config(config), processor(processor), upgraders(upgraders) {
                if (processor == nullptr) {
                    std::string msg = "Processor can not be a nullptr when invoking this constructor.";
                    loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                HttpConnection::initializeStaticPaths(config, &static_paths);
        }

        uint8_t onConnection(int conn_fd);
};

};

#endif