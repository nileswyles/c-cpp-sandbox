#include "http.h"
#include "paths.h"
#include <iostream>
#include <stdio.h>

#include "parser/keyvalue/parse.h"
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

using namespace WylesLibs;
using namespace WylesLibs::Http;
using namespace WylesLibs::Parser;

#define HTTP_FIELD_MAX 64

static Url parseUrl(Reader * reader) {
    Url url;
    // path = /aklmdla/aslmlamk(?)
    Array<uint8_t> path = reader->readUntil("? ");
    if ((char)path.back() == '?') {
        // query = key=value&key2=value2
        url.query_map = KeyValue::parse(reader, '&');
    }
    // TODO: same as other comments... need to fix this...
    url.path = path.removeBack().removeBack().toString();

    return url;
}

void HttpConnection::parseRequest(HttpRequest * request, Reader * reader) {
    if (request == NULL || reader == NULL) {
        throw std::runtime_error("lol....");
    }
    request->method = reader->readUntil(" ").removeBack().toString();
    request->url = parseUrl(reader);
    request->version = reader->readUntil("\n").removeBack().toString();

    request->content_length = -1;
    int field_idx = 0; 
    ReaderTaskDisallow name_operation("\t ");
    ReaderTaskLC lowercase;
    name_operation.nextOperation = &lowercase;
    while (field_idx < HTTP_FIELD_MAX) {
        Array<uint8_t> field_name_array = reader->readUntil(":\n", &name_operation);
        // if (field_name_array.back() == '\n') {
        //     break;
        // }
        // TODO: again this makes no sense.
        if (field_name_array.size() >= 2 && field_name_array.buf()[field_name_array.size()-2] == '\n') {
            printf("FOUND EMPTY NEW LINE AFTER PARSING FIELDS\n");
            break;
        }
        std::string field_name = field_name_array.removeBack().toString();
        ReaderTaskDisallow value_operation("\t ");
        // if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name)) {
        //     value_operation.nextOperation = &lowercase;
        // }
        field_idx++;
        if (field_name == "Content-Length") {
            double value = 0;
            size_t count = 0;
            reader->readNatural(value, count);
            if (count == 0) {
                request->content_length = -1;
            } else {
                request->content_length = (size_t)value;
            }
            reader->readUntil(",\n");
        } else {
            std::string field_value;
            char delimeter = 0x00;
            while (delimeter != '\n') {
                Array<uint8_t> field_value_array = reader->readUntil(",\n", &value_operation);
                delimeter = field_value_array.back();
                // TODO: this makes no sense...
                // lol... 
                if (field_value_array.size() >= 2) {
                    delimeter = field_value_array.buf()[field_value_array.size()-2];
                }
                field_value = field_value_array.removeBack().toString();
                request->fields[field_name].append(field_value);
            }
        }
    }
    if (field_idx == HTTP_FIELD_MAX) {
        throw std::runtime_error("Too many fields in request.");
    }
    if (request->method == "POST" && request->content_length != -1) {
        if ("application/json" == request->fields["content-type"].front()) {
            size_t i = 0;
            request->json_content = Json::parse(reader, i);
        } else if ("application/x-www-form-urlencoded" == request->fields["content-type"].front()) {
            request->form_content = KeyValue::parse(reader, '&');
        } else if ("multipart/formdata" == request->fields["content-type"].front()) {
            // at 128Kb/s can transfer just under 2Mb (bits...) in 15s.
            //  if set min transfer rate at 128Kb/s, 
            //  timeout = content_length*8/SERVER_MINIMUM_CONNECTION_SPEED (bits/bps) 
            serverSetConnectionTimeout(reader->fd(), request->content_length * 8 / SERVER_MINIMUM_CONNECTION_SPEED);
            Multipart::FormData::parse(reader, request->files, request->form_content);
        } else if ("multipart/byteranges" == request->fields["content-type"].front()) {
        } else {
            request->content = reader->readBytes(request->content_length);
        }
    }
}

// might need to change return type...
bool HttpConnection::handleWebsocketRequest(int conn_fd, HttpRequest * request) {
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

                    char message_digest[1024]; // this an issue? TODO: how to calculate buffer size... similarly, validate sec-websocket-key
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
                    response.fields["Sec-WebSocket-Accept"] = std::string(message_digest);

                    BIO_free_all(b64);

                    // API not stable? lol...
                    // EVP_EncodeBlock((unsigned char *)encodedData, (const unsigned char *)checksum, strlen(checksum));
                    std::string response_string = response.toString();
                    write(conn_fd, response_string.c_str(), response_string.size());

                    upgrader->onConnection(conn_fd);

                    // NOTE: the upgrade function should indefinetly block until the connection is intended to be closed.
                    upgraded = 1;
                }
            }
        }
    }
    return upgraded;
}

bool HttpConnection::handleStaticRequest(int conn_fd, HttpRequest * request) {
    bool handled = false;
    std::string path;
    if (request->url.path == "/") {
	    path = Paths::join(this->config.static_path, this->config.root_html_file);
	} else {
        path = Paths::join(this->config.static_path, request->url.path);
    }
    std::string content_type = this->static_paths[path];
	if (content_type != "") {
        HttpResponse response;
		if (request->method == "HEAD" || request->method == "GET ") {
            Array<uint8_t> file_data = File::read(path);
            char content_length[17];
			sprintf(content_length, "%ld", file_data.size());
            response.fields["Content-Length"] = std::string(content_length);
			response.fields["Content-Type"] = content_type;
            // 
			if (request->method == "GET ") {
                printf("GET REQUEST?\n");
				response.content = file_data;
            }
			response.status_code = "200";
		} else {
            response.status_code = "500";
        }
        std::string response_string = response.toString();
        write(conn_fd, response_string.c_str(), response_string.size());
        loggerPrintf(LOGGER_DEBUG, "Wrote static response: \n%s\n", response_string.c_str());
        handled = true;
	}

    return handled;
}

HttpResponse * HttpConnection::requestDispatcher(HttpRequest * request) {
    // for login filters, initializing auth context, etc
    for (size_t i = 0; i < this->request_filters.size(); i++) {
        this->request_filters[i](request);
    }
    HttpResponse * response = nullptr;
    // lol, should I even bother with content-type?
    //  let's leave it there until I start building an actual API... might not be as useful as once thought.
    std::string content_type = request->fields["content-type"].front();
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

uint8_t HttpConnection::onConnection(int conn_fd) {
    HttpRequest request;
    Reader reader(conn_fd);
    HttpResponse * response = nullptr;
    try {
        parseRequest(&request, &reader);
        bool handled = handleStaticRequest(conn_fd, &request);
        if (!handled) {
            handled = handleWebsocketRequest(conn_fd, &request);
        }
        if (!handled) {
            // these should always produce a response, so sig need not include connection file descriptor..
            if (this->processor == nullptr) {
                response = this->requestDispatcher(&request);
            } else {
                response = this->processor(&request);
            }
            if (response == nullptr) {
                std::string msg = "HttpResponse object is a nullptr.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            } else {
                std::string response_string = response->toString();
                delete response;
                free(response);
                int ret = write(conn_fd, response_string.c_str(), response_string.size());
                if (ret == -1) {
                    loggerPrintf(LOGGER_DEBUG, "Error writing to connection file descriptor. Did the connection timer expire?: %d\n", errno);
                }
            }
        }
    } catch (const std::exception& e) {
        delete response;
        free(response);

        // respond with empty HTTP status code 500
        HttpResponse err;
        std::string err_string = err.toString();
        int ret = write(conn_fd, err_string.c_str(), err_string.size());
        if (ret == -1) {
            loggerPrintf(LOGGER_DEBUG, "Error writing to connection file descriptor. Did the connection timer expire?: %d\n", errno);
        }

        loggerPrintf(LOGGER_ERROR, "Exception thrown while processing request: %s\n", e.what());
    }

    close(conn_fd); // doc's say you shouldn't retry close so ignore ret

    return 1;
}
