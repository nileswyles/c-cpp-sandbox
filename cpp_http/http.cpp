#include "http.h"

using namespace WylesLibs;
using namespace WylesLibs::Http;

static Url parseUrl(Reader) {
    Url url;
    // path = /aklmdla/aslmlamk
    // query = ?key=value&key2=value2
    url.path = reader->readUntil("?\n", byteOperation).toString();

    // parse query string...
    char delimeter = value.back();
    if (delimeter == "?") {
        char delimeter = 0x00;
        while (delimeter != '\n') {
            std::string field_name = reader->readUntil('=', IO::byteOperationToLowerCaseAndIgnoreWhiteSpace).toString();
            std::string field_value = reader->readUntil("&\n", byteOperation).popBack().toString();

            request->query_map[field_name] = field_value;
            delimeter = value.back();
        }
    }
    request->fields[field_name].push_back(value.substr(0, value.size()-1));

    return url;
}

void HttpConnection::parseRequest(HttpRequest * request, IO::Reader * reader) {
    if (request == NULL || reader == NULL) {
        throw std::runtime_error("lol....");
    }
    // TODO:
    // also, we don't want to include until character in HttpRequest vars
    request->method = reader->readUntil(' ').toString(); // TODO: error check... if NULL, then read error


    // hmm.....
    ByteOperationIgnore ignore_and_lowercase("\r\n\t ");
    ByteOperationLC lowercase();
    ignore_and_lowercase.next(&lowercase);

    std::string field_name = reader->readUntil(':', &ignore_and_lowercase).toString();
    request->url = Url(request->path);
    request->version = reader->readUntil('\n').toString();

    request->content_length = -1;
    int field_idx = 0; 
    while (field_idx < FIELD_MAX) {
        std::string field_name = reader->readUntil(":\n", IO::byteOperationToLowerCaseAndIgnoreWhiteSpace).toString();
        if (field_name.back() == '\n') {
            break;
        }
        // hmmm... need to trim too... lol
        //  is it ignore all whitespace or just that before and after first and last character? 
        
        //   so trim or ignore? 
        //   they are technically different? lol or rather ignore also implies trimming?

        // LMAO.... WOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
        ByteOperation byteOperation = IO::byteOperationIgnoreWhiteSpace;
        if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name)) {
            byteOperation = IO::byteOperationToLowerCaseAndIgnoreWhiteSpace;
        }
        if (FIELD_VALUES_TO_SPLIT.contains(field_name)) {
            char delimeter = 0x00;
            while (delimeter != '\n') {
                std::string value = reader->readUntil(",\n", byteOperation).toString();
                request->fields[field_name].push_back(value.substr(0, value.size()-1));
                delimeter = value.back();
            }
        } else {
            request->fields[field_name].push_back(reader->readUntil('\n', byteOperation).toString());
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


uint8_t HttpConnection::onConnection(int conn_fd) {
    HttpRequest * request = new HttpRequest();
    IO:Reader * reader = new IO:Reader(conn_fd);
    try {
        parseRequest(request, reader);
        request.print();

        if (request.fields["upgrade"].contains("websocket") && request.fields["connection"].contains("upgrade")) {
            Array<std::string> protocols = request.fields["sec-websocket-protocol"];
            for (size_t i = 0; i < protocols.size(); i++) {
                for (size_t x = 0; i < upgraders.size(); i++) {
                    std::string protocol = protocols[i];
                    if upgrader.path == request.url.path && protocol == upgrader.protocol {
                        response.StatusCode = 101
                        response.fields["Upgrade"] = "websocket"
                        response.fields["Connection"] = "upgrade"
                        response.fields["Sec-Websocket-Protocol"] = protocol
                        response.fields["Sec-WebSocket-Version"] = "13"
                        response.fields["Sec-WebSocket-Extensions"] = ""

                        magic_key := response.Fields["sec-websocket-key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
                        hash := sha1.New()
                        hash.Write([]byte(magic_key))
                        checksum := hash.Sum(nil)
                        dst := make([]byte, base64.StdEncoding.EncodedLen(len(checksum)))
                        base64.StdEncoding.Encode(dst, checksum)
                        response.Fields["Sec-WebSocket-Accept"] = string(dst)

                        if err := utils.WriteBytes(c, response.Bytes()); err != nil {
                            panic(err)
                        }

                        upgrader.onConnection(conn_fd);

                        // NOTE: the upgrade function should indefinetly block until the connection is intended to be closed.
                        return
                    }
                }
            }
        }

        delete reader;
        this->processor(request);
    } catch (const std::exception& e) {
        // Again, low overhead.... so, should be fine...
        std::cout << "Exception: \n" << e.what() << '\n';
    }
    delete request;
    delete reader;

    close(conn_fd); // doc's say you shouldn't retry close so ignore ret

    return 1;
}