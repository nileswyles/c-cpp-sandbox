#include "http.h"
#include "paths.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "parser/keyvalue/parse.h"
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

using namespace WylesLibs;
using namespace WylesLibs::Http;
using namespace WylesLibs::Parser;

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_HTTP
#define LOGGER_LEVEL_HTTP GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_HTTP
#define LOGGER_HTTP 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_HTTP
#include "logger.h"

static Url parseUrl(IOStream * io) {
    Url url;
    // path = /aklmdla/aslmlamk(?)
    SharedArray<uint8_t> path = io->readUntil("? ");
    if ((char)path.back() == '?') {
        // query = key=value&key2=value2
        url.query_map = KeyValue::parse(io, '&');
    }

    // TODO: because removeBack functionality of the array class isn't working...
    //  IOStream can probably use string for readuntil but let's roll with this for now.
    //  Hesitant for obvious reasons...
    std::string pathString = path.toString();
    url.path = pathString.substr(0, pathString.size()-1);

    return url;
}

void HttpConnection::parseRequest(HttpRequest * request, IOStream * io) {
    if (request == NULL || io == NULL) {
        throw std::runtime_error("lol....");
    }
    request->method = io->readUntil(" ").removeBack().toString();
    request->method = request->method.substr(0, request->method.size()-1);

    request->url = parseUrl(io);

    request->version = io->readUntil("\n").removeBack().toString();
    request->version = request->version.substr(0, request->version.size()-1);

    request->content_length = -1;
    int field_idx = 0; 
    while (field_idx < HTTP_FIELD_MAX) {
        std::string field_name = io->readUntil(":\n", &this->whitespace_lc_chain).toString();
        if (field_name[field_name.size()-1] == '\n') {
            printf("FOUND EMPTY NEW LINE AFTER PARSING FIELDS\n");
            break;
        }
        field_name = field_name.substr(0, field_name.size()-1);

        loggerPrintf(LOGGER_DEBUG, "field_name: '%s'\n", field_name.c_str());

        ReaderTaskChain * chain = &this->whitespace_chain;
        if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name.c_str())) {
            chain = &this->whitespace_lc_chain;
        }
        field_idx++;
        char delimeter = 0x00;
        while (delimeter != '\n') {
            std::string field_value = io->readUntil(",\n", chain).toString();
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

    if (request->method == "POST" && request->fields["content-type"].size() > 0 && request->content_length != -1) {
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
            serverSetConnectionTimeout(io->fd, request->content_length * 8 / SERVER_MINIMUM_CONNECTION_SPEED);
            Multipart::FormData::parse(io, request->files, request->form_content);
        } else if ("multipart/byteranges" == request->fields["content-type"].front()) {
        } else {
            request->content = io->readBytes(request->content_length);
        }
    }
}

void HttpConnection::processRequest(IOStream * io, HttpRequest * request) {
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
        if (this->processor == nullptr) {
            response = this->requestDispatcher(request);
        } else {
            response = this->processor(request);
        }
        if (response == nullptr) {
            std::string msg = "HttpResponse object is a nullptr.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        } else {
            writeResponse(response, io);
            return;
        }
}

HttpResponse * HttpConnection::handleStaticRequest(HttpRequest * request) {
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "HANDLING STATIC REQUEST\n");
    HttpResponse * response = nullptr;
    std::string path;
    if (request->url.path == "/") {
	    path = Paths::join(this->config.static_path, this->config.root_html_file);
	} else {
        path = Paths::join(this->config.static_path, request->url.path);
    }
    pthread_mutex_lock(this->static_paths.getMutex());
    std::string content_type = this->static_paths[path];
	if (content_type != "") {
        response = new HttpResponse;
		if (request->method == "HEAD" || request->method == "GET") {
            SharedArray<uint8_t> file_data = this->file_manager->read(path);
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
    pthread_mutex_unlock(this->static_paths.getMutex());
    return response;
}

bool HttpConnection::handleWebsocketRequest(IOStream * io, HttpRequest * request) {
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "HANDLING WEBSOCKET REQUEST\n");
    bool upgraded = 0;
    if (request->fields["upgrade"].contains("websocket") && request->fields["connection"].contains("upgrade")) {
        SharedArray<std::string> protocols = request->fields["sec-websocket-protocol"];
        for (size_t i = 0; i < protocols.size(); i++) {
            for (size_t x = 0; i < this->upgraders.size(); i++) {
                std::string protocol = protocols[i];
                ConnectionUpgrader * upgrader = this->upgraders[i];
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
                    SHA_CTX context;
                    int result = SHA1_Init(&context);
                    result = SHA1_Update(&context, (void *)key_string.c_str(), key_string.size());
                    result = SHA1_Final((unsigned char *)message_digest, &context);

                    char base64_encoded[32] = {0};
                    EVP_EncodeBlock((unsigned char *)base64_encoded, (unsigned char *)message_digest, 20);

                    response->fields["Sec-WebSocket-Accept"] = std::string(base64_encoded);

                    this->writeResponse(response, io);

                    serverDisableConnectionTimeout(io->fd);
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
HttpResponse * HttpConnection::handleTimeoutRequests(IOStream * io, HttpRequest * request) {
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "HANDLING TIMEOUT REQUEST\n");
    HttpResponse * response = nullptr;
    if (request->url.path == "/timeout" && request->method == "POST") {
        JsonObject * obj = (JsonObject *)request->json_content;
        if (obj != nullptr) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Keys: %lu\n", obj->keys.size());
            for (size_t i = 0; i < obj->keys.size(); i++) {
                std::string key = obj->keys.at(i);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Key: %s\n", key.c_str());
                JsonValue * value = obj->values.at(i);
                if (key == "socket") {
                    serverSetInitialSocketTimeout(io->fd, (uint32_t)setVariableFromJsonValue<double>(value));
                } else if (key == "connection") {
                    serverSetInitialConnectionTimeout(io->fd, (uint32_t)setVariableFromJsonValue<double>(value));
                }
            }

            std::string responseJson("{\"socket\":");
            char timeout[11];
            sprintf(timeout, "%d", serverGetSocketTimeout(io->fd));
            responseJson += timeout;

            responseJson += ",\"connection\":";
            sprintf(timeout, "%d", serverGetConnectionTimeout(io->fd));
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
        sprintf(timeout, "%d", serverGetSocketTimeout(io->fd));
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
        sprintf(timeout, "%d", serverGetConnectionTimeout(io->fd));
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

HttpResponse * HttpConnection::requestDispatcher(HttpRequest * request) {
    // for login filters, initializing auth context, etc
    for (size_t i = 0; i < this->request_filters.size(); i++) {
        this->request_filters[i](request);
    }
    HttpResponse * response = nullptr;
    // lol, should I even bother with content-type?
    //  let's leave it there until I start building an actual API... might not be as useful as once thought.
    std::string content_type;
    if (request->fields["content-type"].size() > 0) {
        content_type = request->fields["content-type"].front();
    }
    if (this->request_map[request->url.path].contains("")) {
        // indicates that the user does not care about content type.
        response = this->request_map[request->url.path][""](request);
    } else if (this->request_map[request->url.path].contains(content_type)) {
        response = this->request_map[request->url.path][content_type](request);
    }
    for (size_t i = 0; i < this->response_filters.size(); i++) {
        this->response_filters[i](response);
    }
    return response;
}

SSL * HttpConnection::acceptTLS(int fd) {
    if (this->context == nullptr) {
        throw std::runtime_error("Server SSL Context isn't initialized. Check server configuration.");
    } else {
        SSL * ssl = SSL_new(this->context);
        if (ssl == nullptr) {
            throw std::runtime_error("Error initializing SSL object for connection.");
        }

        int verify_mode = SSL_VERIFY_NONE;
        if (this->config.client_auth_enabled) {
            verify_mode = SSL_VERIFY_PEER;
        }
        SSL_set_verify(ssl, verify_mode, nullptr);
        SSL_set_accept_state(ssl);

        SSL_set_fd(ssl, fd);

        SSL_clear_mode(ssl, 0);
        // SSL_MODE_AUTO_RETRY

        // TODO: so apparently this is optional lol... SSL_read will perform handshake...
        //  also, investigate renegotiation, auto retry...
        int accept_result = SSL_accept(ssl);
        loggerPrintf(LOGGER_DEBUG, "ACCEPT RESULT: %d\n", accept_result);
        loggerPrintf(LOGGER_DEBUG, "MODE: %lx, VERSION: %s, IS SERVER: %d\n", SSL_get_mode(ssl), SSL_get_version(ssl), SSL_is_server(ssl));
        loggerExec(LOGGER_DEBUG, SSL_SESSION_print_fp(stdout, SSL_get_session(ssl)););
        if (accept_result != 1) {
            int error_code = SSL_get_error(ssl, accept_result) + 0x30;
            // SSL_ERROR_NONE
            throw std::runtime_error("SSL handshake failed. ERROR CODE: " + std::string((char *)&error_code));
        } // connection accepted if accepted_result == 1

        return ssl;
    }
}

uint8_t HttpConnection::onConnection(int fd) {
    // ! IMPORTANT -
    //  expanding on thoughts on mallocs/new vs stack
    //  so, if need access to more memory you can call new where needed at point of creation of each thread.

    HttpRequest request;
    IOStream io(fd);
    try {
        if (this->config.tls_enabled) {
            io.ssl = this->acceptTLS(fd); // initializes ssl object for connection
        }
        this->parseRequest(&request, &io);
        loggerPrintf(LOGGER_DEBUG, "Request path: '%s', method: '%s'\n", request.url.path.c_str(), request.method.c_str());
        this->processRequest(&io, &request);
    } catch (const std::exception& e) {
        loggerPrintf(LOGGER_ERROR, "Exception thrown while processing request: %s\n", e.what());
        if (this->config.tls_enabled && io.ssl == nullptr) {
            // then deduce error occured while initializing ssl
            loggerPrintf(LOGGER_DEBUG, "Error accepting and configuring TLS connection.\n");
        } else {
            // respond with empty HTTP status code 500
            HttpResponse * err = new HttpResponse;
            writeResponse(err, &io);
        }
    }

    if (io.ssl != nullptr) {
        SSL_shutdown(io.ssl);
        SSL_free(io.ssl);
    }
    close(fd); // doc's say you shouldn't retry close so ignore ret

    return 1;
}

void HttpConnection::writeResponse(HttpResponse * response, IOStream * io) {
    std::string data = response->toString();
    // ! IMPORTANT - this response pointer will potentially come from an end-user (another developer)... YOU WILL ENCOUNTER PROBLEMS IF THE POINTER IS CREATED USING MALLOC AND NOT NEW
    delete response;

    if (io->writeBuffer((void *)data.c_str(), data.size()) == -1) {
        loggerPrintf(LOGGER_DEBUG, "Error writing to connection: %d\n", errno);
    } else {
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Wrote response to connection: \n%s\n", data.c_str());
    }
}