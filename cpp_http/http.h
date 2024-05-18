#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include "reader.h"

#include "../c_lib/server.h"

// TODO: how to keep imports clean? don't want to rely on preprocessor stuff.?

#define HTTP_FIELD_MAX 64

using namespace WylesLibs;

namespace WylesLibs::Http {
// 
typedef void(RequestProcessor)(HttpRequest&);

static Array<std::string> FIELD_VALUES_TO_SPLIT{
    "connection",
	"sec-websocket-protocol",
	"accept-encoding"
};

static Array<std::string> FIELD_VALUES_TO_LOWER_CASE{
    "connection",
    "upgrade"
};

class Url {
    public:
        std::string path;
        std::unordered_map<std::string, std::string> query_map;
};

class HttpRequest {
    public:
        std::string method;
        Url url;
        std::string version;
        
        std::unordered_map<std::string, Array<std::string>> fields;
        std::unordered_map<std::string, Array<std::string>> cookies;
        Array<uint8_t> content;

        ~HttpRequest() {
            // free(r->method);
            // free(r->path);
            // free(r->version);
            // for (int i = 0; i < FIELD_MAX; i++) {
            //     free(r->fields[i].name);
            //     free(r->fields[i].value);
            // }
            // free(r->content);
        }

        void print() {
            printf("%s %s %s\n", this->method, this->path, this->version);

            int idx = 0;
            std::string field_name = this->fields[idx].name;
            while(field_name != "" && idx < FIELD_MAX) {
                // TODO: add field for cookies
                // TODO: add a separate field to request struct for this?
                printf("%s: %s\n", field_name, this->fields[idx].value);
                idx++;
                field_name = this->fields[idx].name;
            }

            if (this->content.size() != 0 && this->content_length > 0) {
                printf("---CONTENT_START(%d)---\n", this->content_length);
                for (int i = 0; i < this->content_length; i++) {
                    printf("%c", (char)this->content[i]);
                }
            }
            printf("---END\n");
        }
};

class HttpResponse {
    public:
        std::string method;
        std::string version;
        
        // // hmm... lol... yeah.... 
        // HttpUrl url;

        std::unordered_map<std::string, Array<std::string>> fields;
        Array<uint8_t> content;

        ~HttpResponse() {
            // free(r->method);
            // free(r->path);
            // free(r->version);
            // for (int i = 0; i < FIELD_MAX; i++) {
            //     free(r->fields[i].name);
            //     free(r->fields[i].value);
            // }
            // free(r->content);
        }

        void print() {
            printf("%s %s %s\n", this->method, this->path, this->version);

            int idx = 0;
            std::string field_name = this->fields[idx].name;
            while(field_name != "" && idx < FIELD_MAX) {
                // TODO: add field for cookies
                // TODO: add a separate field to request struct for this?
                printf("%s: %s\n", field_name, this->fields[idx].value);
                idx++;
                field_name = this->fields[idx].name;
            }

            if (this->content.size() != 0 && this->content_length > 0) {
                printf("---CONTENT_START(%d)---\n", this->content_length);
                for (int i = 0; i < this->content_length; i++) {
                    printf("%c", (char)this->content[i]);
                }
            }
            printf("---END\n");
        }
};

// voila!
class HttpConnection {
    private:
        RequestProcessor processor;
        Array<ConnectionUpgrader> upgraders;
        void parseRequest(HttpRequest * request, IO:Reader * reader);
    public:
        HttpConnection(RequestProcessor processor, Array<ConnectionUpgrader> upgraders): processor(processor), upgraders(upgraders) {}
        uint8_t onConnection(int conn_fd);
};

};
