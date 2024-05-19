#include "http.h"

using namespace WylesLibs;
using namespace WylesLibs::Http;

static Url parseUrl(Reader * reader) {
    Url url;
    // path = /aklmdla/aslmlamk
    // query = ?key=value&key2=value2
    Array<uint8_t> path = reader->readUntil("?\n");
    if (path.back() == "?") {
        char delimeter = 0x00;
        while (delimeter != '\n') {
            Array<uint8_t> field_name = reader->readUntil('=');
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

void HttpConnection::parseRequest(HttpRequest * request, IO::Reader * reader) {
    if (request == NULL || reader == NULL) {
        throw std::runtime_error("lol....");
    }

    // hm.... think about reader interface/func signatures again...
    request->method = reader->readUntil(' ', false).toString();
    request->url = parseUrl(reader);
    request->version = reader->readUntil('\n', false).toString();

    request->content_length = -1;
    int field_idx = 0; 
    ByteOperationIgnore name_operation("\r\n\t ");
    ByteOperationLC lowercase();
    name_operation.next(&lowercase);
    while (field_idx < FIELD_MAX) {
        Array<uint8_t> field_name = reader->readUntil('=', &name_operation);
        if (field_name.back() == '\n') {
            break;
        }
        ByteOperationIgnore value_operation("\r\n\t ");
        if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name)) {
            value_operation.next(&lowercase);
        }
        char delimeter = 0x00;
        while (delimeter != '\n') {
            Array<uint8_t> field_value = reader->readUntil(",\n", &value_operation);
            delimeter = field_value.back();
            request->fields[field_name.popBack().toString()] = field_value.popBack().toString();
        }
        field_idx++;
        if (field_name == "Content-Length") {
            // TODO: negatives? use strtoul instead?
            request->content_length = atoi(value);
        }
    }
    if (field_idx == FIELD_MAX) {
        throw std::runtime_error("Too many fields in request.");
    }
    if (request->content_length != -1) {
        // printf("request->content: %lx\n", request->content);
        request->content = reader->readBytes(request->content_length);
        // printf("request->content: %lx\n", request->content);
    }
}

bool HttpConnection::handleWebsocketRequest(int conn_fd, HttpRequest request) {
    bool upgraded = false;
    if (request.fields["upgrade"].contains("websocket") && request.fields["connection"].contains("upgrade")) {
        Array<std::string> protocols = request.fields["sec-websocket-protocol"];
        for (size_t i = 0; i < protocols.size(); i++) {
            for (size_t x = 0; i < this->upgraders.size(); i++) {
                std::string protocol = protocols[i];
                ConnectionUpgrader upgrader = this->upgraders[i];

                if upgrader.path == request.url.path && protocol == upgrader.protocol {
                    printf("Websocket upgrade request...")
                    // response.StatusCode = 101
                    // response.fields["Upgrade"] = "websocket"
                    // response.fields["Connection"] = "upgrade"
                    // response.fields["Sec-Websocket-Protocol"] = protocol
                    // response.fields["Sec-WebSocket-Version"] = "13"
                    // response.fields["Sec-WebSocket-Extensions"] = ""

                    // magic_key := response.Fields["sec-websocket-key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
                    // hash := sha1.New()
                    // hash.Write([]byte(magic_key))
                    // checksum := hash.Sum(nil)
                    // dst := make([]byte, base64.StdEncoding.EncodedLen(len(checksum)))
                    // base64.StdEncoding.Encode(dst, checksum)
                    // response.Fields["Sec-WebSocket-Accept"] = string(dst)

                    // if err := utils.WriteBytes(c, response.Bytes()); err != nil {
                    //     panic(err)
                    // }

                    upgrader.onConnection(conn_fd);

                    // NOTE: the upgrade function should indefinetly block until the connection is intended to be closed.
                    upgraded = true;
                }
            }
        }
    }
    return upgraded;
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