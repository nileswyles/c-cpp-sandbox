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

#include "key_generator.h"
#include "web/http/http_types.h"
#include "memory/pointers.h"
#include "estream/estream.h"
#include "estream/reader_task.h"
#include "estream/byteestream.h"
#include "web/server.h"
#include "config.h"
#include "connection.h"
#include "web/authorization.h"
#include "parser/multipart/parse_formdata.h"
#include "web/http/http_file_watcher.h"
#include "thread_safe_map.h"
#include "file/file.h"
#include "paths.h"
#include "datastructures/array.h"

// #ifndef WYLESLIBS_HTTP_DEBUG
// #define WYLESLIBS_HTTP_DEBUG 0
// #endif

#define HTTP_FIELD_MAX 64

using namespace WylesLibs;
using namespace WylesLibs::Paths;
using namespace WylesLibs::File;

namespace WylesLibs::Http {

static SharedArray<const char *> FIELD_VALUES_TO_LOWER_CASE{
    "connection",
    "upgrade"
};

class HttpServer: public Server {
    private:
        static void initializeEStreamTasks(ReaderTaskDisallow<SharedArray<uint8_t>>& whitespace_chain, ReaderTaskDisallow<SharedArray<uint8_t>>& whitespace_lc_chain, ReaderTaskLC<SharedArray<uint8_t>>& lowercase_task) {
            lowercase_task = ReaderTaskLC<SharedArray<uint8_t>>();
            whitespace_chain.to_disallow = "\t ";
            whitespace_lc_chain.to_disallow = "\t ";
            whitespace_lc_chain.next_operation = &lowercase_task;
        }

        static void initializeStaticPaths(HttpServerConfig config, ThreadSafeMap<std::string, std::string>& static_paths) {
            loggerPrintf(LOGGER_DEBUG, "Static Paths: %s\n", config.static_path.c_str());
            for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(config.static_path)) {
                std::string path = dir_entry.path().string();
                static_paths[path] = contentTypeFromPath(path);
            }
        }

        static void initializeSSLContext(HttpServerConfig config, SSL_CTX *& context) {
            if (config.tls_enabled) {
                // context might not be a per connection thing... 
                context = SSL_CTX_new(TLS_method());
                if (context == nullptr) {
                    loggerPrintf(LOGGER_DEBUG, "Error initializing SSL Context.");
                } else {
                    int result = -1;
                    if (config.client_auth_enabled) {
                        result = SSL_CTX_load_verify_file(context, config.path_to_trust_chain_cert.c_str());
                        if (result != 1) {
                            std::string msg = "Failed configuration of verify file."; 
                            loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                            throw std::runtime_error(msg);
                        }
                        result = SSL_CTX_use_certificate_chain_file(context, config.path_to_trust_chain_cert.c_str()); // TODO: redundant? create client auth test to confirm.
                        if (result != 1) {
                            std::string msg = "Failed configuration of certificate chain file."; 
                            loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                            throw std::runtime_error(msg);
                        }
                    }
                    result = -1;
                    result = SSL_CTX_use_certificate_file(context, config.path_to_cert.c_str(), SSL_FILETYPE_PEM);
                    if (result != 1) {
                        std::string msg = "Failed configuration of certificate file.";
                        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                        throw std::runtime_error(msg);
                    }
                    result = -1;
                    result = SSL_CTX_use_PrivateKey_file(context, config.path_to_private_key.c_str(), SSL_FILETYPE_PEM);
                    if (result != 1) {
                        std::string msg = "Failed configuration of private key file.";
                        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                        throw std::runtime_error(msg);
                    }
                }
            } else {
                context = nullptr;
            }
        }

        // prerequisites,
        //  to be initialized
        static void construct(HttpServerConfig config, ESharedPtr<FileManager> file_manager,

                                            ThreadSafeMap<std::string, std::string>& static_paths,
                                            SSL_CTX *& ssl_context,
                                            ReaderTaskDisallow<SharedArray<uint8_t>>& whitespace_chain,
                                            ReaderTaskDisallow<SharedArray<uint8_t>>& whitespace_lc_chain,
                                            ReaderTaskLC<SharedArray<uint8_t>>& lowercase_task,
                                            UniqueKeyGenerator& key_generator) {
            initializeEStreamTasks(whitespace_chain, whitespace_lc_chain, lowercase_task);
            initializeStaticPaths(config, static_paths);
            initializeSSLContext(config, ssl_context);
            key_generator = UniqueKeyGenerator(config, UniqueKeyGeneratorStore(file_manager, Paths::join(config.resources_root, "sequence_store")));
            // Array<std::string> paths{config.static_path};
            //     file_watcher = ESharedPtr<HttpFileWatcher>(new HttpFileWatcher(config, &static_paths, paths, &static_paths_mutex));
            //     file_watcher->initialize(file_watcher);
        }

    public:
        RequestProcessor * processor;
        std::map<std::string, RequestProcessor *> request_map;
        SharedArray<RequestFilter> request_filters;
        SharedArray<ResponseFilter> response_filters;
        SharedArray<ConnectionUpgrader *> upgraders;
        ThreadSafeMap<std::string, std::string> static_paths; 

        SSL_CTX * ssl_context;

        ESharedPtr<HttpFileWatcher> file_watcher;

        ReaderTaskDisallow<SharedArray<uint8_t>> whitespace_chain;
        ReaderTaskDisallow<SharedArray<uint8_t>> whitespace_lc_chain;
        ReaderTaskLC<SharedArray<uint8_t>> lowercase_task;

        HttpServerConfig config;

        HttpServer() = default;
        HttpServer(HttpServerConfig config, 
                        std::map<std::string, RequestProcessor *> request_map, 
                        SharedArray<RequestFilter> request_filters, SharedArray<ResponseFilter> response_filters, 
                        SharedArray<ConnectionUpgrader *> upgraders, ESharedPtr<FileManager> file_manager) {

            loggerExec(LOGGER_DEBUG_VERBOSE,
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Request Paths: \n");
                for (auto p: WylesLibs::Http::requestMap) {
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", p.first.c_str());
                }
            )
            this->config = config;
            this->request_map = request_map;
            this->request_filters = request_filters;
            this->response_filters = response_filters;
            this->upgraders = upgraders;
            this->processor = nullptr;
            this->file_manager = file_manager;
            construct(config, 
                        this->file_manager,
                        this->static_paths, 
                        this->ssl_context, 
                        this->whitespace_chain, 
                        this->whitespace_lc_chain,
                        this->lowercase_task,
                        this->key_generator);
        }
        HttpServer(HttpServerConfig config, 
                        RequestProcessor * processor, 
                        SharedArray<ConnectionUpgrader *> upgraders, 
                        ESharedPtr<FileManager> file_manager) {
            if (processor == nullptr) {
                std::string msg = "Processor can not be a nullptr when invoking this constructor.";
                loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            this->config = config;
            this->processor = processor;
            this->upgraders = upgraders;
            this->file_manager = file_manager;
            construct(config, 
                        this->file_manager,
                        this->static_paths, 
                        this->ssl_context, 
                        this->whitespace_chain, 
                        this->whitespace_lc_chain,
                        this->lowercase_task,
                        this->key_generator);
        }
        ~HttpServer() {
            if (this->ssl_context != nullptr) {
                SSL_CTX_free(this->ssl_context);
            }
        }
        int onConnection(int fd) override;
};

// @ static

// assuming amd64 - what year are we in? LMAO
// static_assert(sizeof(HttpServer) == 
//     sizeof(RequestProcessor *) +
//     sizeof(map<std::string, map<std::string, RequestProcessor *>>) +
//     sizeof(SharedArray<RequestFilter>) + 
//     sizeof(SharedArray<ResponseFilter>) + 
//     sizeof(SharedArray<ConnectionUpgrader *>) + 
//     sizeof(HttpServerConfig) +
//     sizeof(ThreadSafeMap<std::string, std::string>) +
//     8 + // sizeof(SSL_CTX) +
//     sizeof(ESharedPtr<HttpFileWatcher>) +
//     sizeof(ESharedPtr<FileManager>) +
//     sizeof(ReaderTaskDisallow<SharedArray<uint8_t>>) +
//     sizeof(ReaderTaskDisallow<SharedArray<uint8_t>>) +
//     sizeof(ReaderTaskLC<SharedArray<uint8_t>>)
// );
// static_assert(sizeof(HttpServer) == 704);

};

#endif