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

#ifndef HTTP_DEBUG
#define HTTP_DEBUG 0
#endif

#define HTTP_FIELD_MAX 64

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
    Array<uint8_t> path = io->readUntil("? ");
    if ((char)path.back() == '?') {
        // query = key=value&key2=value2
        url.query_map = KeyValue::parse(io, '&');
    }
    // TODO: same as other comments... need to fix this...
    url.path = path.removeBack().removeBack().toString();

    return url;
}

void HttpConnection::parseRequest(HttpRequest * request, IOStream * io) {
    if (request == NULL || io == NULL) {
        throw std::runtime_error("lol....");
    }
    request->method = io->readUntil(" ").removeBack().removeBack().toString();
    request->url = parseUrl(io);
    request->version = io->readUntil("\n").removeBack().removeBack().toString();

    request->content_length = -1;
    int field_idx = 0; 
    ReaderTaskDisallow name_operation("\t ");
    ReaderTaskLC lowercase;
    name_operation.nextOperation = &lowercase;
    while (field_idx < HTTP_FIELD_MAX) {
        Array<uint8_t> field_name_array = io->readUntil(":\n", &name_operation);
        // if (field_name_array.back() == '\n') {
        //     break;
        // }
        // TODO: again this makes no sense.
        if (field_name_array.size() >= 2 && field_name_array.buf()[field_name_array.size()-2] == '\n') {
            printf("FOUND EMPTY NEW LINE AFTER PARSING FIELDS\n");
            break;
        }
        std::string field_name = field_name_array.removeBack().removeBack().toString();
        ReaderTaskDisallow value_operation("\t ");
        if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name.c_str())) {
            value_operation.nextOperation = &lowercase;
        }
        field_idx++;
        std::string field_value;
        char delimeter = 0x00;
        while (delimeter != '\n') {
            Array<uint8_t> field_value_array = io->readUntil(",\n", &value_operation);
            delimeter = field_value_array.back();
            // TODO: this makes no sense...
            // lol... 
            if (field_value_array.size() >= 2) {
                delimeter = field_value_array.buf()[field_value_array.size()-2];
            }
            field_value = field_value_array.removeBack().removeBack().toString();
            request->fields[field_name].append(field_value);
            if (field_name == "content-length") {
                request->content_length = atoi(field_value.c_str());
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
        bool upgradedConnectionToWebsocket = this->handleWebsocketRequest(io, request);
        if (upgradedConnectionToWebsocket) {
            return;
        }

        HttpResponse * response = this->handleStaticRequest(request); 
        if (response != nullptr) {
            this->writeResponse(response, io);
            return;
        } 

        if (HTTP_DEBUG) {
            response = this->handleTimeoutRequests(io, request);
            if (response != nullptr) {
                this->writeResponse(response, io);
                return;
            }
        }

        // these should always produce a response, so sig need not include connection file descriptor..
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

bool HttpConnection::handleWebsocketRequest(IOStream * io, HttpRequest * request) {
    bool upgraded = 0;
    if (request->fields["upgrade"].contains("websocket") && request->fields["connection"].contains("upgrade")) {
        Array<std::string> protocols = request->fields["sec-websocket-protocol"];
        for (size_t i = 0; i < protocols.size(); i++) {
            for (size_t x = 0; i < this->upgraders.size(); i++) {
                std::string protocol = protocols[i];
                ConnectionUpgrader * upgrader = this->upgraders[i];

                if (upgrader->path == request->url.path && protocol == upgrader->protocol) {
                    printf("Websocket upgrade request...");
                    HttpResponse response;

                    response.status_code = "101"; 
                    response.fields["Upgrade"] = "websocket";
                    response.fields["Connection"] = "upgrade";
                    response.fields["Sec-Websocket-Protocol"] = protocol;
                    response.fields["Sec-WebSocket-Version"] = "13";
                    response.fields["Sec-WebSocket-Extensions"] = "";

                    // not sure what this is doing, but if that's what the browser wants lmao..
                    std::string key_string = request->fields["sec-websocket-key"].toString() + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

                    char message_digest[20];
                    SHA_CTX context;
                    int result = SHA1_Init(&context);
                    result = SHA1_Update(&context, (void *)key_string.c_str(), key_string.size());
                    result = SHA1_Final((unsigned char *)message_digest, &context);

                    BIO *bio, *b64;
                    b64 = BIO_new(BIO_f_base64());
                    char base64_encoded[2048];
                    bio = BIO_new_mem_buf(base64_encoded, 2048);
                    BIO_push(b64, bio);
                    BIO_write(b64, message_digest, strlen(message_digest));

                    printf(base64_encoded);
                    printf(message_digest);
                    response.fields["Sec-WebSocket-Accept"] = std::string(message_digest);
                    BIO_flush(b64);
                    printf(base64_encoded);
                    printf(message_digest);
                    // response.fields["Sec-WebSocket-Accept"] = std::string(message_digest);

                    BIO_free_all(b64);

                    // // API not stable? lol...
                    // // EVP_EncodeBlock((unsigned char *)encodedData, (const unsigned char *)checksum, strlen(checksum));
                    this->writeResponse(&response, io);

                    upgrader->onConnection(io);

                    // NOTE: the upgrade function should indefinetly block until the connection is intended to be closed.
                    upgraded = 1;
                }
            }
        }
    }
    return upgraded;
}

HttpResponse * HttpConnection::handleStaticRequest(HttpRequest * request) {
    HttpResponse * response = nullptr;
    std::string path;
    if (request->url.path == "/") {
	    path = Paths::join(this->config.static_path, this->config.root_html_file);
	} else {
        path = Paths::join(this->config.static_path, request->url.path);
    }
    std::string content_type = this->static_paths[path];
	if (content_type != "") {
        response = new HttpResponse;
		if (request->method == "HEAD" || request->method == "GET") {
            Array<uint8_t> file_data = File::read(path);
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

HttpResponse * HttpConnection::handleTimeoutRequests(IOStream * io, HttpRequest * request) {
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

uint8_t HttpConnection::onConnection(int fd) {
    HttpRequest request;
    HttpResponse * response = nullptr;
    SSL * ssl = nullptr;
    IOStream * io = nullptr;
    try {
        // TODO: might not need the tls_enabled flag?
        if (this->config.tls_enabled) {
            ssl = this->acceptTLS(fd); // initializes ssl object for connection
            IOStream sslIO = IOStream(ssl);
            io = (IOStream *)&sslIO;
        } else {
            IOStream regularIO(fd);
            io = &regularIO;
        }

        this->parseRequest(&request, io);
        loggerPrintf(LOGGER_DEBUG, "Request path: '%s', method: '%s'\n", request.url.path.c_str(), request.method.c_str());
        this->processRequest(io, &request);
    } catch (const std::exception& e) {
        if (ssl == nullptr) {
            loggerPrintf(LOGGER_DEBUG, "Error accepting and configuring TLS connection.\n");
        } else {
            // respond with empty HTTP status code 500
            HttpResponse err;
            this->writeResponse(&err, io);
        }
        loggerPrintf(LOGGER_ERROR, "Exception thrown while processing request: %s\n", e.what());
    }

    if (ssl != nullptr) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    close(fd); // doc's say you shouldn't retry close so ignore ret

    return 1;
}

void HttpConnection::writeResponse(HttpResponse * response, IOStream * io) {
    std::string data = response->toString();
    delete response;
    free(response);

    if (io->writeBuffer((void *)data.c_str(), data.size()) == -1) {
        loggerPrintf(LOGGER_DEBUG, "Error writing to connection: %d\n", errno);
    }
}

SSL * HttpConnection::acceptTLS(int fd) {
    if (this->context == nullptr) {
        throw std::runtime_error("Server SSL Context isn't initialized. Check server configuration.");
    } else {
        SSL * ssl = SSL_new(context);
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

        //  investigate renegotiation, auto retry...
        // TODO: so apparently this is optional lol... SSL_read will perform handshake...
        int accept_result = SSL_accept(ssl);
        loggerPrintf(LOGGER_DEBUG, "ACCEPT RESULT: %d\n", accept_result);
        loggerPrintf(LOGGER_DEBUG, "MODE: %lx, VERSION: %s, IS SERVER: %d\n", SSL_get_mode(ssl), SSL_get_version(ssl), SSL_is_server(ssl));
        loggerExec(LOGGER_DEBUG, SSL_SESSION_print_fp(stdout, SSL_get_session(ssl)););
        if (accept_result != 1) {
            int error_code = SSL_get_error(ssl, accept_result) + 0x30;
            // SSL_ERROR_NONE
            throw std::runtime_error("SSL handshake failed but don't care about specific error at the moment. ERROR CODE: " + std::string((char *)&error_code));
        } // connection accepted if accepted_result == 1
        // else {
            // uint8_t buf[8096];
            // int ret = SSL_read(ssl, buf, 8096);
            // loggerPrintf(LOGGER_DEBUG, "WTF is even this?\n");
            // loggerPrintByteArray(LOGGER_DEBUG, buf, ret);
        // }

        return ssl;
    }
}