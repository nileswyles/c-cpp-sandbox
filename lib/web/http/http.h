#ifndef WYLESLIB_HTTP_H
#define WYLESLIB_HTTP_H

// glibc
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

// cpp-stdlib
#include <filesystem>
#include <unordered_map>
#include <map>
#include <memory>

// other
#include <openssl/ssl.h>

#include "iostream/iostream.h"
#include "web/server.h"
#include "config.h"
#include "connection.h"
#include "web/authorization.h"
#include "parser/multipart/parse_formdata.h"
#include "parser/multipart/multipart_file.h"
#include "web/http/http_file_watcher.h"
#include "thread_safe_map.h"
#include "file.h"
#include "paths.h"
#include "datastructures/array.h"

// #ifndef WYLESLIBS_HTTP_DEBUG
// #define WYLESLIBS_HTTP_DEBUG 0
// #endif

#define HTTP_FIELD_MAX 64

using namespace WylesLibs;
using namespace WylesLibs::Parser::Multipart;
using namespace WylesLibs::Paths;

namespace WylesLibs::Http {

static SharedArray<const char *> FIELD_VALUES_TO_LOWER_CASE{
    "connection",
    "upgrade"
};

typedef struct Url {
    std::string path;
    std::unordered_map<std::string, std::string> query_map;
} Url;

class HttpRequest {
    public:
        std::string method;
        Url url;
        std::string version;
        
        std::map<std::string, SharedArray<std::string>> fields;
        std::map<std::string, SharedArray<std::string>> cookies;

        size_t content_length;
        JsonValue * json_content;
        SharedArray<MultipartFile> files;
        std::unordered_map<std::string, std::string> form_content;
        SharedArray<uint8_t> content;

        Authorization auth;

        HttpRequest(): json_content(nullptr) {}
        ~HttpRequest() = default;
};

class HttpResponse {
    public:
        std::string status_code;
        std::string method;
        std::string version;
        std::unordered_map<std::string, std::string> fields;
        std::unordered_map<std::string, std::string> cookies;
        SharedArray<uint8_t> content;

        HttpResponse(): version("HTTP/1.1"), status_code("500") {
			fields["Connection"].append("close");
        }
        ~HttpResponse() = default;

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
        SharedArray<RequestFilter> request_filters;
        SharedArray<ResponseFilter> response_filters;
        SharedArray<ConnectionUpgrader *> upgraders;
        HttpServerConfig config;
        ThreadSafeMap<std::string, std::string> static_paths; 

        SSL_CTX * context;

        std::shared_ptr<HttpFileWatcher> file_watcher;
        void parseRequest(HttpRequest * request, IOStream * reader);
        void processRequest(IOStream * io, HttpRequest * request);

        HttpResponse * handleStaticRequest(HttpRequest * request);
        bool handleWebsocketRequest(IOStream * io, HttpRequest * request);
#ifdef WYLESLIBS_HTTP_DEBUG
        HttpResponse * handleTimeoutRequests(IOStream * io, HttpRequest * request);
#endif
        HttpResponse * requestDispatcher(HttpRequest * request);

        SSL * acceptTLS(int conn_fd);
        ReaderTaskDisallow whitespace_chain;
        ReaderTaskDisallow whitespace_lc_chain;
        ReaderTaskLC lowercase_task;
        void writeResponse(HttpResponse * response, IOStream * io);

        void initializeStaticPaths(HttpServerConfig config, ThreadSafeMap<std::string, std::string> static_paths) {
            loggerPrintf(LOGGER_DEBUG, "Static Paths: %s\n", config.static_path.c_str());
                for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(config.static_path)) {
                    std::string path = dir_entry.path().string();
                    static_paths[path] = contentTypeFromPath(path);
                }
        }
        void initializeSSLContext() {
            if (this->config.tls_enabled) {
                // context might not be a per connection thing... 
                this->context = SSL_CTX_new(TLS_method());
                if (this->context == nullptr) {
                    loggerPrintf(LOGGER_DEBUG, "Error initializing SSL Context.");
                } else {
                    int result = -1;
                    if (this->config.client_auth_enabled) {
                        result = SSL_CTX_load_verify_file(context, this->config.path_to_trust_chain_cert.c_str());
                        if (result != 1) {
                            std::string msg = "Failed configuration of verify file."; 
                            loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                            throw std::runtime_error(msg);
                        }
                        result = SSL_CTX_use_certificate_chain_file(context, this->config.path_to_trust_chain_cert.c_str()); // TODO: redundant? create client auth test to confirm.
                        if (result != 1) {
                            std::string msg = "Failed configuration of certificate chain file."; 
                            loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                            throw std::runtime_error(msg);
                        }
                    }
                    result = -1;
                    result = SSL_CTX_use_certificate_file(context, this->config.path_to_cert.c_str(), SSL_FILETYPE_PEM);
                    if (result != 1) {
                        std::string msg = "Failed configuration of certificate file.";
                        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                        throw std::runtime_error(msg);
                    }
                    result = -1;
                    result = SSL_CTX_use_PrivateKey_file(context, this->config.path_to_private_key.c_str(), SSL_FILETYPE_PEM);
                    if (result != 1) {
                        std::string msg = "Failed configuration of private key file.";
                        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                        throw std::runtime_error(msg);
                    }
                }
            } else {
                this->context = nullptr;
            }
        }
        void initializeIOStreamTasks() {
            this->lowercase_task = ReaderTaskLC();
            this->whitespace_chain.to_disallow = "\t ";
            this->whitespace_lc_chain.to_disallow = "\t ";
            this->whitespace_lc_chain.nextOperation = &this->lowercase_task;
        }
    public:
        HttpConnection() = default;
        // haha, funny how that worked out...
        HttpConnection(HttpServerConfig pConfig, map<std::string, map<std::string, RequestProcessor *>> pRequest_map, 
                        SharedArray<RequestFilter> pRequest_filters, SharedArray<ResponseFilter> pResponse_filters, 
                        SharedArray<ConnectionUpgrader *> pUpgraders) {
            config = pConfig;
            request_map = pRequest_map;
            request_filters = pRequest_filters;
            response_filters = pResponse_filters;
            upgraders = pUpgraders;
            processor = nullptr;
        }
        HttpConnection(HttpServerConfig config, RequestProcessor * processor, SharedArray<ConnectionUpgrader *> upgraders): 
            config(config), processor(processor), upgraders(upgraders) {
                if (processor == nullptr) {
                    std::string msg = "Processor can not be a nullptr when invoking this constructor.";
                    loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
        }
        ~HttpConnection() = default;

        uint8_t onConnection(int conn_fd);

        // ! IMPORTANT - this needs to be explicitly called by construction caller because CPP.
        void initialize() {
            initializeIOStreamTasks();
            initializeStaticPaths(config, static_paths);
            initializeSSLContext();
            // Array<std::string> paths{config.static_path};
        //     file_watcher = std::make_shared<HttpFileWatcher>(config, &static_paths, paths, &static_paths_mutex);
        //     file_watcher->initialize(file_watcher);
        }
};

// @ static

// assuming amd64 - what year are we in? LMAO
static_assert(sizeof(Url) == 
    sizeof(std::string) + 
    sizeof(std::unordered_map<std::string, std::string>)
); 
static_assert(sizeof(Url) == 88);

static_assert(sizeof(HttpConnection) == 
    sizeof(RequestProcessor *) +
    sizeof(map<std::string, map<std::string, RequestProcessor *>>) +
    sizeof(SharedArray<RequestFilter>) + 
    sizeof(SharedArray<ResponseFilter>) + 
    sizeof(SharedArray<ConnectionUpgrader *>) + 
    sizeof(HttpServerConfig) +
    sizeof(ThreadSafeMap<std::string, std::string>) +
    8 + // sizeof(SSL_CTX) +
    sizeof(std::shared_ptr<HttpFileWatcher>) +
    sizeof(ReaderTaskDisallow) +
    sizeof(ReaderTaskDisallow) +
    sizeof(ReaderTaskLC)
);
static_assert(sizeof(HttpConnection) == 688);

};

#endif