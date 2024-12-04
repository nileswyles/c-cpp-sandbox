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

#include "memory/pointers.h"
#include "estream/estream.h"
#include "estream/reader_task.h"
#include "estream/byteestream.h"
#include "web/server.h"
#include "config.h"
#include "connection.h"
#include "web/authorization.h"
#include "parser/multipart/parse_formdata.h"
#include "parser/multipart/multipart_file.h"
#include "web/http/http_file_watcher.h"
#include "thread_safe_map.h"
#include "file/file.h"
#include "paths.h"
#include "datastructures/array.h"

#include "web/http/http_connection_etask.h"

// #ifndef WYLESLIBS_HTTP_DEBUG
// #define WYLESLIBS_HTTP_DEBUG 0
// #endif

#define HTTP_FIELD_MAX 64

using namespace WylesLibs;
using namespace WylesLibs::Parser::Multipart;
using namespace WylesLibs::Paths;
using namespace WylesLibs::File;

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
        ESharedPtr<JsonValue> json_content;
        SharedArray<MultipartFile> files;
        std::unordered_map<std::string, std::string> form_content;
        SharedArray<uint8_t> content;

        Authorization auth;

        HttpRequest() = default;
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

// TODO: this is kind of a behemoth of a class/file, might or might not be necessary.

// voila!
class HttpServer: public Server {
    private:
        RequestProcessor * processor;
        map<std::string, map<std::string, RequestProcessor *>> request_map;
        SharedArray<RequestFilter> request_filters;
        SharedArray<ResponseFilter> response_filters;
        SharedArray<ConnectionUpgrader *> upgraders;
        HttpServerConfig config;
        ThreadSafeMap<std::string, std::string> static_paths; 

        SSL_CTX * context;

        ESharedPtr<HttpFileWatcher> file_watcher;
        ESharedPtr<FileManager> file_manager;

        void parseRequest(HttpRequest * request, ByteEStream * reader);
        void processRequest(ByteEStream * io, HttpRequest * request);

        HttpResponse * handleStaticRequest(HttpRequest * request);
        bool handleWebsocketRequest(ByteEStream * io, HttpRequest * request);
#ifdef WYLESLIBS_HTTP_DEBUG
        HttpResponse * handleTimeoutRequests(ByteEStream * io, HttpRequest * request);
#endif
        HttpResponse * requestDispatcher(HttpRequest * request);

        ReaderTaskDisallow<SharedArray<uint8_t>> whitespace_chain;
        ReaderTaskDisallow<SharedArray<uint8_t>> whitespace_lc_chain;
        ReaderTaskLC<SharedArray<uint8_t>> lowercase_task;
        void writeResponse(HttpResponse * response, ByteEStream * io);

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
        void initializeEStreamTasks() {
            this->lowercase_task = ReaderTaskLC<SharedArray<uint8_t>>();
            this->whitespace_chain.to_disallow = "\t ";
            this->whitespace_lc_chain.to_disallow = "\t ";
            this->whitespace_lc_chain.next_operation = &this->lowercase_task;
        }

        friend HttpConnectionETask; // lol.
    public:
        HttpServer() = default;
        // haha, funny how that worked out...
        HttpServer(HttpServerConfig pConfig, map<std::string, map<std::string, RequestProcessor *>> pRequest_map, 
                        SharedArray<RequestFilter> pRequest_filters, SharedArray<ResponseFilter> pResponse_filters, 
                        SharedArray<ConnectionUpgrader *> pUpgraders, ESharedPtr<FileManager> file_manager) {
            config = pConfig;
            request_map = pRequest_map;
            request_filters = pRequest_filters;
            response_filters = pResponse_filters;
            upgraders = pUpgraders;
            processor = nullptr;
            file_manager = file_manager;
        }
        HttpServer(HttpServerConfig config, RequestProcessor * processor, SharedArray<ConnectionUpgrader *> upgraders, ESharedPtr<FileManager> file_manager): 
            config(config), processor(processor), upgraders(upgraders), file_manager(file_manager) {
                if (processor == nullptr) {
                    std::string msg = "Processor can not be a nullptr when invoking this constructor.";
                    loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
        }
        ~HttpServer() {
            if (this->context != nullptr) {
                SSL_CTX_free(this->context);
            }
        }

        // ! IMPORTANT - this needs to be explicitly called by construction caller because CPP.
        void initialize() {
            initializeEStreamTasks();
            initializeStaticPaths(config, static_paths);
            initializeSSLContext();
            // Array<std::string> paths{config.static_path};
            //     file_watcher = ESharedPtr<HttpFileWatcher>(new HttpFileWatcher(config, &static_paths, paths, &static_paths_mutex));
            //     file_watcher->initialize(file_watcher);
        }

        void onConnection(int fd) override;
};

// @ static

// assuming amd64 - what year are we in? LMAO
static_assert(sizeof(Url) == 
    sizeof(std::string) + 
    sizeof(std::unordered_map<std::string, std::string>)
); 
static_assert(sizeof(Url) == 88);

static_assert(sizeof(HttpServer) == 
    sizeof(RequestProcessor *) +
    sizeof(map<std::string, map<std::string, RequestProcessor *>>) +
    sizeof(SharedArray<RequestFilter>) + 
    sizeof(SharedArray<ResponseFilter>) + 
    sizeof(SharedArray<ConnectionUpgrader *>) + 
    sizeof(HttpServerConfig) +
    sizeof(ThreadSafeMap<std::string, std::string>) +
    8 + // sizeof(SSL_CTX) +
    sizeof(ESharedPtr<HttpFileWatcher>) +
    sizeof(ESharedPtr<FileManager>) +
    sizeof(ReaderTaskDisallow<SharedArray<uint8_t>>) +
    sizeof(ReaderTaskDisallow<SharedArray<uint8_t>>) +
    sizeof(ReaderTaskLC<SharedArray<uint8_t>>)
);
// static_assert(sizeof(HttpServer) == 704);

};

#endif