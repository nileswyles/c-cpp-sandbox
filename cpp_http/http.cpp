#include "http.h"
#include <iostream>
// #include <openssl/sha.h>

using namespace WylesLibs;
using namespace WylesLibs::Http;

#define HTTP_FIELD_MAX 64

static Url parseUrl(Reader * reader) {
    Url url;
    // path = /aklmdla/aslmlamk
    // query = ?key=value&key2=value2
    Array<uint8_t> path = reader->readUntil("?\n");
    if ((char)path.back() == '?') {
        char delimeter = 0x00;
        while (delimeter != '\n') {
            Array<uint8_t> field_name = reader->readUntil("=");
            Array<uint8_t> field_value = reader->readUntil("&\n");
            delimeter = field_value.back();
            // alternatively, can toString then remove last?
            //  this is probably more performant... the append is guaranteed to not allocate and only iterates string once (creation?)...
            url.query_map[field_name.popBack().toString()] = field_value.popBack().toString();
        }
    }
    // hmm... this stringyness is the shiznit... son! #streams...
    url.path = path.popBack().toString();

    return url;
}

void HttpConnection::parseRequest(HttpRequest * request, Reader * reader) {
    if (request == NULL || reader == NULL) {
        throw std::runtime_error("lol....");
    }
    request->method = reader->readUntil(" ", true).toString();
    request->url = parseUrl(reader);
    request->version = reader->readUntil("\n", true).toString();

    request->content_length = -1;
    int field_idx = 0; 
    ByteOperationIgnore name_operation("\t ");
    ByteOperationLC lowercase;
    name_operation.nextOperation = &lowercase;
    while (field_idx < HTTP_FIELD_MAX) {
        Array<uint8_t> field_name_array = reader->readUntil("=", &name_operation);
        if (field_name_array.back() == '\n') {
            break;
        }
        std::string field_name = field_name_array.popBack().toString();
        ByteOperationIgnore value_operation("\t ");
        // TODO:
        // if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name)) {
        //     value_operation.nextOperation = &lowercase;
        // }
        std::string field_value;
        char delimeter = 0x00;
        while (delimeter != '\n') {
            Array<uint8_t> field_value_array = reader->readUntil(",\n", &value_operation);
            delimeter = field_value_array.back();
            field_value = field_value_array.popBack().toString();
            request->fields[field_name].append(field_value);
        }
        field_idx++;
        if (field_name == "Content-Length") {
            // TODO: negatives? use strtoul instead?
            request->content_length = atoi(field_value.c_str());
        }
    }
    if (field_idx == HTTP_FIELD_MAX) {
        throw std::runtime_error("Too many fields in request.");
    }
    if (request->content_length != -1) {
        // printf("request->content: %lx\n", request->content);
        request->content = reader->readBytes(request->content_length);
        // printf("request->content: %lx\n", request->content);
    }
}

// might need to change return type...
bool HttpConnection::handleWebsocketRequest(int conn_fd, HttpRequest * request) {
    bool upgraded = false;
    if (request->fields["upgrade"].contains("websocket") && request->fields["connection"].contains("upgrade")) {
        Array<std::string> protocols = request->fields["sec-websocket-protocol"];
        for (size_t i = 0; i < protocols.size(); i++) {
            for (size_t x = 0; i < this->upgraders.size(); i++) {
                std::string protocol = protocols[i];
                ConnectionUpgrader * upgrader = this->upgraders[i];

                if (upgrader->path == request->url.path && protocol == upgrader->protocol) {
                    printf("Websocket upgrade request...");
                    HttpResponse response;

                    response.status_code = 101; 
                    response.fields["Upgrade"] = "websocket";
                    response.fields["Connection"] = "upgrade";
                    response.fields["Sec-Websocket-Protocol"] = protocol;
                    response.fields["Sec-WebSocket-Version"] = "13";
                    response.fields["Sec-WebSocket-Extensions"] = "";

                    // unsigned char *SHA1(const unsigned char *data, size_t count, unsigned char *md_buf);
                    // not sure what this is doing, but if that's what the browser wants lmao..
                    std::string key_string = request->fields["sec-websocket-key"].toString() + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                    // char * checksum = SHA1(key_string, key_string.size(), nullptr);
                    // // char * implies NUL terminated right?
                    // char encodedData[1024]; // this an issue? TODO: how to calculate buffer size... similarly, validate sec-websocket-key
                    // // then base64 encode checksum...
                    // EVP_EncodeBlock(encodedData, checksum, strlen(checksum));
                    // printf(encodedData);
                    // response.fields["Sec-WebSocket-Accept"] = std::string(encodedData);

                    std::string response_string = response.toString();
                    write(conn_fd, response_string.c_str(), response_string.size());

                    upgrader->onConnection(conn_fd);

                    // NOTE: the upgrade function should indefinetly block until the connection is intended to be closed.
                    upgraded = true;
                }
            }
        }
    }
    return upgraded;
}

bool HttpConnection::handleStaticRequest(int conn_fd, HttpRequest * request) {
    // if (this->config.root_html_file != "" && request.url.path == "/") {
	// 	request.url.path = s.config.root_html_file;
	// }

	// path := filepath.Join(s.config.PathToStaticDir, request.Url.Path)
	// if (this->static_paths[path] != "") {
	// 	s.processRequestToStaticPath(request, response, path)
	// } else { // else process handlers
    //     return false;




	// 	for _, f := range s.request_handlers {
	// 		if err = f(request, response); err != nil {
	// 			response.StatusCode = 500
	// 		}
	// 	}
	// }
    return false;
}

uint8_t HttpConnection::onConnection(int conn_fd) {
    HttpRequest * request = new HttpRequest();
    Reader * reader = new Reader(conn_fd);
    try {
        parseRequest(request, reader);
        if (!handleWebsocketRequest(conn_fd, request)) {
            this->processor(request);
        }
        delete reader;
    } catch (const std::exception& e) {
        // Again, low overhead.... so, should be fine...
        std::cout << "Exception: \n" << e.what() << '\n';
    }
    delete request;
    delete reader;

    close(conn_fd); // doc's say you shouldn't retry close so ignore ret

    return 1;
}