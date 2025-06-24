#include "web/http/http_connection_etask.h"
#include "web/http/http.h"
#include "estream/byteestream.h"
#include "parser/keyvalue/parse.h"

#include "paths.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_HTTP_CONNECTION_ETASK
#define LOGGER_LEVEL_HTTP_CONNECTION_ETASK GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_HTTP_CONNECTION_ETASK
#define LOGGER_HTTP_CONNECTION_ETASK 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP_CONNECTION_ETASK

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_HTTP_CONNECTION_ETASK
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Http;
using namespace WylesLibs::Parser;

void HttpConnectionETask::processRequest(ByteEStream * io, HttpRequest * request) {
        HttpResponse * response = this->handleStaticRequest(request); 
        if (response != nullptr) {
            this->writeResponse(response, io);
            return;
        } 

        bool upgradedConnectionToWebsocket = this->handleWebsocketRequest(io, request);
        if (upgradedConnectionToWebsocket) {
            return;
        }

#ifdef WYLESLIBS_HTTP_DEBUG
        response = this->handleTimeoutRequests(io, request);
        if (response != nullptr) {
            this->writeResponse(response, io);
            return;
        }
#endif

        loggerPrintf(LOGGER_DEBUG_VERBOSE, "HANDLING REQUEST\n");
        if (this->server->processor == nullptr) {
            response = this->requestDispatcher(request);
        } else {
            response = this->server->processor(request);
        }
        if (response == nullptr) {
            std::string msg = "HttpResponse object is a nullptr.";
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        } else {
            this->writeResponse(response, io);
            return;
        }
}

HttpResponse * HttpConnectionETask::handleStaticRequest(HttpRequest * request) {
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "HANDLING STATIC REQUEST\n");
    HttpResponse * response = nullptr;
    std::string path;
    if (request->url.path == "/") {
	    path = Paths::join(this->server->config.static_path, this->server->config.root_html_file);
	} else {
        path = Paths::join(this->server->config.static_path, request->url.path);
    }
    pthread_mutex_lock(this->server->static_paths.getMutex());
    std::string content_type = this->server->static_paths[path];
    pthread_mutex_unlock(this->server->static_paths.getMutex());
	if (content_type != "") {
        response = new HttpResponse;
		if (request->method == "HEAD" || request->method == "GET") {
            SharedArray<uint8_t> file_data = ESHAREDPTR_GET_PTR(this->server->file_manager)->read(path);
            char content_length[17];
			sprintf(content_length, "%ld", file_data.size());
            response->fields["Content-Length"] = std::string(content_length);
			response->fields["Content-Type"] = content_type;
			if (request->method == "GET") {
				response->content = file_data;
            }
			response->status_code = "200";
		} else {
            response->status_code = "500";
        }
	}
    return response;
}

bool HttpConnectionETask::handleWebsocketRequest(ByteEStream * io, HttpRequest * request) {
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "HANDLING WEBSOCKET REQUEST\n");
    bool upgraded = 0;
    if (request->fields["upgrade"].contains("websocket") && request->fields["connection"].contains("upgrade")) {
        SharedArray<std::string> protocols = request->fields["sec-websocket-protocol"];
        for (size_t i = 0; i < protocols.size(); i++) {
            for (size_t x = 0; i < this->server->upgraders.size(); i++) {
                std::string protocol = protocols[i];
                ConnectionUpgrader * upgrader = this->server->upgraders[i];
                if (upgrader->path == request->url.path && protocol == upgrader->protocol) {
                    HttpResponse * response = new HttpResponse;

                    response->status_code = "101"; 
                    response->fields["Upgrade"] = "websocket";
                    response->fields["Connection"] = "upgrade";
                    response->fields["Sec-Websocket-Protocol"] = protocol;
                    response->fields["Sec-WebSocket-Version"] = "13";
                    response->fields["Sec-WebSocket-Extensions"] = "";

                    // not sure what this is doing, but if that's what the browser wants lmao..
                    printf("sec-websocket-key: %s\n", request->fields["sec-websocket-key"][0].c_str());
                    std::string key_string = request->fields["sec-websocket-key"].at(0) + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

                    char message_digest[20];
                    SHA_CTX ssl_context;
                    int result = SHA1_Init(&ssl_context);
                    if (0 == result) { throw std::runtime_error("SHA1 Init failed."); }
                    result = SHA1_Update(&ssl_context, (void *)key_string.c_str(), key_string.size());
                    if (0 == result) { throw std::runtime_error("SHA1 Update failed."); }
                    result = SHA1_Final((unsigned char *)message_digest, &ssl_context);
                    if (0 == result) { throw std::runtime_error("SHA1 Final failed."); }

                    char base64_encoded[32] = {0};
                    EVP_EncodeBlock((unsigned char *)base64_encoded, (unsigned char *)message_digest, 20);

                    response->fields["Sec-WebSocket-Accept"] = std::string(base64_encoded);

                    this->writeResponse(response, io);

                    this->server->disableConnectionTimeout(io->fd);
                    nice(-7);
                    upgrader->onConnection(io);

                    // NOTE: the upgrade function should indefinetly block until the connection is intended to be closed.
                    upgraded = 1;
                }
            }
        }
    }
    return upgraded;
}

#ifdef WYLESLIBS_HTTP_DEBUG
HttpResponse * HttpConnectionETask::handleTimeoutRequests(ByteEStream * io, HttpRequest * request) {
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "HANDLING TIMEOUT REQUEST\n");
    HttpResponse * response = nullptr;
    if (request->url.path == "/timeout" && request->method == "POST") {
        JsonObject * obj = (JsonObject *)ESHAREDPTR_GET_PTR(request->json_content);
        if (obj != nullptr) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = obj->values.at(i);
                if (key == "socket") {
                   this->server->setInitialSocketTimeout(io->fd, (uint32_t)setVariableFromJsonValue<double>(value));
                } else if (key == "connection") {
                   this->server->setInitialConnectionTimeout(io->fd, (uint32_t)setVariableFromJsonValue<double>(value));
                }
            }

            std::string responseJson("{\"socket\":");
            char timeout[11];
            sprintf(timeout, "%d",this->server->getSocketTimeout(io->fd));
            responseJson += timeout;

            responseJson += ",\"connection\":";
            sprintf(timeout, "%d",this->server->getConnectionTimeout(io->fd));
            responseJson += timeout;
            responseJson += "}";
     
            response = new HttpResponse;
            response->status_code = "200";
            Array<uint8_t> content;
            content.append((uint8_t *)responseJson.data(), responseJson.size());
            response->content = content;
        }
    } else if (request->url.path == "/timeout/socket" && request->method == "GET") {
        std::string responseJson("{\"socket\":");
        char timeout[11];
        sprintf(timeout, "%d",this->server->getSocketTimeout(io->fd));
        responseJson += timeout;
        responseJson += "}";

        response = new HttpResponse;
        response->status_code = "200";
        Array<uint8_t> content;
        content.append((uint8_t *)responseJson.data(), responseJson.size());
        response->content = content;
    } else if (request->url.path == "/timeout/connection" && request->method == "GET") {
        std::string responseJson("{\"connection\":");
        char timeout[11];
        sprintf(timeout, "%d",this->server->getConnectionTimeout(io->fd));
        responseJson += timeout;
        responseJson += "}";

        response = new HttpResponse;
        response->status_code = "200";
        Array<uint8_t> content;
        content.append((uint8_t *)responseJson.data(), responseJson.size());
        response->content = content;
    }
    return response;
}
#endif

HttpResponse * HttpConnectionETask::requestDispatcher(HttpRequest * request) {
    // for login filters, initializing auth ssl_context, etc
    for (size_t i = 0; i < this->server->request_filters.size(); i++) {
        this->server->request_filters[i](request);
    }
    HttpResponse * response = nullptr;

    HttpProcessorItem test_item(*request, nullptr, {}, {});
    // TODO: if there's a more optimal hash function for this specific use-case then pivot to that. 
    if (this->server->request_map.contains(test_item)) {
        HttpProcessorItem actual_item = this->server->request_map[test_item];
        for (size_t i = 0; i < actual_item.request_filters.size(); i++) {
            actual_item.request_filters[i](request);
        }
        response = actual_item.processor(request);
        for (size_t i = 0; i < actual_item.response_filters.size(); i++) {
            actual_item.response_filters[i](response);
        }
    }
    if (response == nullptr) {
        response = new HttpResponse;
    }
    for (size_t i = 0; i < this->server->response_filters.size(); i++) {
        this->server->response_filters[i](response);
    }
    return response;
}

void HttpConnectionETask::writeResponse(HttpResponse * response, ByteEStream * io) {
    std::string data = response->toString();
    // ! IMPORTANT - this response pointer will potentially come from an end-user (another developer)... YOU WILL ENCOUNTER PROBLEMS IF THE POINTER IS CREATED USING MALLOC AND NOT NEW
    delete response;

    if (io->write((uint8_t *)data.c_str(), data.size()) == -1) {
        loggerPrintf(LOGGER_DEBUG, "Error writing to connection: %d\n", errno);
    } else {
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Wrote response to connection: \n%s\n", data.c_str());
    }
}

// alternatively can include all of http's private functions here and just access already public functions 
void HttpConnectionETask::run() {
    ByteEStream eio;
#ifdef WYLESLIBS_SSL_ENABLED
    SSLEStream sslio;
#endif
    ByteEStream * io;
    bool acceptedTLS = false;
    try {
#ifdef WYLESLIBS_SSL_ENABLED
        if (true == this->server->config.tls_enabled) {
            sslio = SSLEStream(this->server->ssl_context, this->fd, this->server->config.client_auth_enabled); // initializes ssl object for connection
            acceptedTLS = true;
            io = dynamic_cast<ByteEStream *>(&sslio);
        } else {
            eio = ByteEStream(this->fd);
            io = &eio;
        }
#else
        eio = ByteEStream(this->fd);
        io = &eio;
#endif
        HttpRequest request(io, this->server);
        loggerPrintf(LOGGER_DEBUG, "Request path: '%s', method: '%s'\n", request.url.path.c_str(), request.method.c_str());
        this->processRequest(io, &request);
    } catch (const std::exception& e) {
        loggerPrintf(LOGGER_INFO, "Exception thrown while processing request: %s\n", e.what());
        if (true == this->server->config.tls_enabled && false == acceptedTLS) {
            // then deduce error occured while initializing ssl
            loggerPrintf(LOGGER_DEBUG, "Error accepting and configuring TLS connection.\n");
        } else {
            // respond with empty HTTP status code 500
            HttpResponse * err = new HttpResponse;
            this->writeResponse(err, io);
        }
    }
}

void HttpConnectionETask::onExit() {
    close(this->fd);
}