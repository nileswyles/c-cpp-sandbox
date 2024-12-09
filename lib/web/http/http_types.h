#ifndef WYLESLIB_HTTP_TYPES_H
#define WYLESLIB_HTTP_TYPES_H

#include <unordered_map>
#include <map>

#include "memory/pointers.h"
#include "web/authorization.h"
#include "datastructures/array.h"
#include "parser/multipart/multipart_file.h"
#include "parser/json/json.h"

using namespace WylesLibs;
using namespace WylesLibs::Parser::Json;

namespace WylesLibs::Http {

typedef struct Url {
    std::string path;
    std::unordered_map<std::string, std::string> query_map;
} Url;

class HttpRequest {
    public:
        std::string method;
        Url url;
        std::string version;
        
        std::map<std::string, SharedArray<std::string>> fields;
        std::map<std::string, SharedArray<std::string>> cookies;

        size_t content_length;
        ESharedPtr<JsonValue> json_content;
        SharedArray<WylesLibs::MultipartFile> files;
        std::unordered_map<std::string, std::string> form_content;
        SharedArray<uint8_t> content;

        Authorization auth;

        HttpRequest() = default;
        ~HttpRequest() = default;
};

class HttpResponse {
    public:
        std::string status_code;
        std::string method;
        std::string version;
        std::unordered_map<std::string, std::string> fields;
        std::unordered_map<std::string, std::string> cookies;
        SharedArray<uint8_t> content;

        HttpResponse(): version("HTTP/1.1"), status_code("500") {
			fields["Connection"].append("close");
        }
        ~HttpResponse() = default;

        std::string toString() {
            // header = this->version + " " + strconv.Itoa(status_code) + " " + status_map[r.StatusCode] + "\n";
            std::string response = this->version + " " + this->status_code + "\n";
            for (const auto& [key, value] : this->fields) {
				response += key + ": " + value + "\n";
			}
            for (const auto& [key, value] : this->cookies) {
				response += "set-cookie: " + value + "\n";
			}
			response += "\n";
            response += content.toString();
            return response;
        }
};

// typedef HttpResponse *(RequestProcessor)(HttpRequest *);
//  vs.
// typedef HttpResponse *(* RequestProcessor)(HttpRequest *);
typedef HttpResponse *(RequestProcessor)(HttpRequest *);
typedef void(* RequestFilter)(HttpRequest *);
typedef void(* ResponseFilter)(HttpResponse *);

// @ static

// assuming amd64 - what year are we in? LMAO
static_assert(sizeof(Url) == 
    sizeof(std::string) + 
    sizeof(std::unordered_map<std::string, std::string>)
); 
static_assert(sizeof(Url) == 88);

};

#endif