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

static Url parseUrl(ByteEStream * io) {
    Url url;
    // path = /aklmdla/aslmlamk(?)
    SharedArray<uint8_t> path = io->read("? ");
    if ((char)path.back() == '?') {
        // query = key=value&key2=value2
        url.query_map = KeyValue::parse(io, '&');
    }

    // TODO: because removeBack functionality of the array class isn't working...
    //  ByteEStream can probably use string for readuntil but let's roll with this for now.
    //  Hesitant for obvious reasons...
    std::string pathString = path.toString();
    url.path = pathString.substr(0, pathString.size()-1);

    return url;
}

void HttpConnectionETask::parseRequest(HttpRequest * request, ByteEStream * io) {
    if (request == NULL || io == NULL) {
        throw std::runtime_error("lol....");
    }
    // TODO: this is terrible as is... stringyness must work.
    request->method = io->read(" ").removeBack().toString();
    request->method = request->method.substr(0, request->method.size()-1);

    request->url = parseUrl(io);

    request->version = io->read("\n").removeBack().toString();
    request->version = request->version.substr(0, request->version.size()-1);

    request->content_length = SIZE_MAX;
    int field_idx = 0; 
    while (field_idx < HTTP_FIELD_MAX) {
        std::string field_name = io->read(":\n", &this->server->whitespace_lc_chain).toString();
        if (field_name[field_name.size()-1] == '\n') {
            printf("FOUND EMPTY NEW LINE AFTER PARSING FIELDS\n");
            break;
        }
        field_name = field_name.substr(0, field_name.size()-1);

        loggerPrintf(LOGGER_DEBUG, "field_name: '%s'\n", field_name.c_str());

        ReaderTaskChain<SharedArray<uint8_t>> * chain = &this->server->whitespace_chain;
        if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name.c_str())) {
            chain = &this->server->whitespace_lc_chain;
        }
        field_idx++;
        char delimeter = 0x00;
        while (delimeter != '\n') {
            std::string field_value = io->read(",\n", chain).toString();
            // if size == 0, throw an exception idc...
            delimeter = (char)field_value[field_value.size()-1];
            // only process field if it's an actual field, else check delimeter in while loop...
            if (field_value.size() >= 2) {
                field_value = field_value.substr(0, field_value.size()-2);

                loggerPrintf(LOGGER_DEBUG, "delimeter: '0x%x', field_value: '%s'\n", delimeter, field_value.c_str());
         
                request->fields[field_name].append(field_value);
                if (field_name == "content-length") {
                    request->content_length = atoi(field_value.c_str());
                }
            }
        }
    }
    if (field_idx == HTTP_FIELD_MAX) {
        throw std::runtime_error("Too many fields in request.");
    }

    if (request->method == "POST" && request->fields["content-type"].size() > 0 && request->content_length != SIZE_MAX) {
        loggerPrintf(LOGGER_DEBUG, "Content-Type: %s, Content-Length: %ld\n", request->fields["content-type"].front().c_str(), request->content_length);
        if ("application/json" == request->fields["content-type"].front()) {
            size_t i = 0;
            request->json_content = Json::parse(io, i);
        } else if ("application/x-www-form-urlencoded" == request->fields["content-type"].front()) {
            request->form_content = KeyValue::parse(io, '&');
        } else if ("multipart/formdata" == request->fields["content-type"].front()) {
            // at 128Kb/s can transfer just under 2Mb (bits...) in 15s.
            //  if set min transfer rate at 128Kb/s, 
            //  timeout = content_length*8/SERVER_MINIMUM_CONNECTION_SPEED (bits/bps) 
            this->server->setConnectionTimeout(io->fd, request->content_length * 8 / SERVER_MINIMUM_CONNECTION_SPEED);
            nice(-4);
            Multipart::FormData::parse(io, request->files, request->form_content, this->server->file_manager);
        } else if ("multipart/byteranges" == request->fields["content-type"].front()) {
        } else {
            request->content = ((EStream<uint8_t> *)io)->read((size_t)request->content_length);
        }
    }
}

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
    // lol, should I even bother with content-type?
    //  let's leave it there until I start building an actual API... might not be as useful as once thought.
    std::string content_type;
    if (request->fields["content-type"].size() > 0) {
        content_type = request->fields["content-type"].front();
    }
    if (this->server->request_map[request->url.path].contains("")) {
        // indicates that the user does not care about content type.
        response = this->server->request_map[request->url.path][""](request);
    } else if (this->server->request_map[request->url.path].contains(content_type)) {
        response = this->server->request_map[request->url.path][content_type](request);
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
    HttpRequest request;
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
        this->parseRequest(&request, io);
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