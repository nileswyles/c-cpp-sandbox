#ifndef WYLESLIB_HTTP_TYPES_H
#define WYLESLIB_HTTP_TYPES_H

#include <unordered_map>
#include <map>

#include "pointers.h"
#include "authorization.h"
#include "server.h"
#include "array.h"
#include "multipart_file.h"
#include "json.h"
#include "byteestream.h"

#include "string_format.h"

using namespace WylesLibs;
using namespace WylesLibs::Parser::Json;

#define HTTP_FIELD_MAX 64

namespace WylesLibs::Http {
    static constexpr const char * CONTENT_TYPE_KEY = "content-type";

    typedef struct Url {
        std::string path;
        std::unordered_map<std::string, std::string> query_map;
    } Url;

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

        // TODO: if lower bounds check...
        url.path = pathString.substr(0, pathString.size()-1);

        return url;
    }

    static void initializeByteStreamReaderTasks(ReaderTaskDisallow<SharedArray<uint8_t>>& whitespace_chain, ReaderTaskDisallow<SharedArray<uint8_t>>& whitespace_lc_chain, ReaderTaskLC<SharedArray<uint8_t>>& lowercase_task) {
        lowercase_task = ReaderTaskLC<SharedArray<uint8_t>>();
        whitespace_chain.to_disallow = "\t ";
        whitespace_lc_chain.to_disallow = "\t ";
        whitespace_lc_chain.next_operation = &lowercase_task;
    }

    class HttpRequest {
        public:
            std::string method;
            Url url;
            std::string backwards_path; // low-hanging fruit optimization of the request match logic.
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
            HttpRequest(std::string path, std::string method, std::string content_type = ""): url(Url()), method(method), fields(std::map<std::string, SharedArray<std::string>>()) {
                url.path = path;
                for (int64_t i = path.size() - 1; i >= 0; i--) {
                    backwards_path += path[i];
                }
                fields[CONTENT_TYPE_KEY] = content_type;
            };
            ~HttpRequest() = default;

            HttpRequest(ByteEStream * io, HttpServer * server = nullptr) {
                ReaderTaskDisallow<SharedArray<uint8_t>> whitespace_chain;
                ReaderTaskDisallow<SharedArray<uint8_t>> whitespace_lc_chain;
                ReaderTaskLC<SharedArray<uint8_t>> lowercase_task;
                initializeByteStreamReaderTasks(whitespace_chain, whitespace_lc_chain, lowercase_task);

                // TODO: this is terrible as is... stringyness must work. lower bounds check if can't get removeBack functionality working.
                method = io->read(" ").removeBack().toString();
                method = method.substr(0, method.size()-1);
      
                url = parseUrl(io);
      
                version = io->read("\n").removeBack().toString();
                version = version.substr(0, version.size()-1);
      
                content_length = SIZE_MAX;
                int field_idx = 0; 
                while (field_idx < HTTP_FIELD_MAX) {
                    std::string field_name = io->read(":\n", &whitespace_lc_chain).toString();
                    if (field_name[field_name.size()-1] == '\n') {
                        printf("FOUND EMPTY NEW LINE AFTER PARSING FIELDS\n");
                        break;
                    }
                    field_name = field_name.substr(0, field_name.size()-1);
      
                    loggerPrintf(LOGGER_DEBUG, "field_name: '%s'\n", field_name.c_str());
      
                    ReaderTaskChain<SharedArray<uint8_t>> * chain = &whitespace_chain;
                    if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name.c_str())) {
                        chain = &whitespace_lc_chain;
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
                     
                            fields[field_name].append(field_value);
                            if (field_name == "content-length") {
                                content_length = atoi(field_value.c_str());
                            }
                        }
                    }
                }
                if (field_idx == HTTP_FIELD_MAX) {
                    throw std::runtime_error("Too many fields in request.");
                }
      
                if (method == "POST" && fields[CONTENT_TYPE_KEY].size() > 0 && content_length != SIZE_MAX) {
                    loggerPrintf(LOGGER_DEBUG, "Content-Type: %s, Content-Length: %ld\n", fields[CONTENT_TYPE_KEY].front().c_str(), content_length);
                    if ("application/json" == fields[CONTENT_TYPE_KEY].front()) {
                        size_t i = 0;
                        json_content = Json::parse(io, i);
                    } else if ("application/x-www-form-urlencoded" == fields[CONTENT_TYPE_KEY].front()) {
                        form_content = KeyValue::parse(io, '&');
                    } else if ("multipart/formdata" == fields[CONTENT_TYPE_KEY].front()) {
                        FileManager file_manager;
                        if (server != nullptr) {
                            // at 128Kb/s can transfer just under 2Mb (bits...) in 15s.
                            //  if set min transfer rate at 128Kb/s, 
                            //  timeout = content_length*8/SERVER_MINIMUM_CONNECTION_SPEED (bits/bps) 
                            server->setConnectionTimeout(io->fd, content_length * 8 / SERVER_MINIMUM_CONNECTION_SPEED);
                            nice(-4);
                            file_manager = server->file_manager;
                        }
                        Multipart::FormData::parse(io, files, form_content, file_manager);
                    } else if ("multipart/byteranges" == fields[CONTENT_TYPE_KEY].front()) {
                    } else {
                        content = ((EStream<uint8_t> *)io)->read((size_t)content_length);
                    }
                } else {
                    content = io->readEls((size_t)content_length);
                }
            }

            bool operator== (HttpRequest& x) {
                std::string this_string_hash = this->backwards_path + this->method;
                std::string x_string_hash = x.backwards_path + x.method;
                if (this->fields[CONTENT_TYPE_KEY].size() > 0) {
                    SharedArray<std::string> this_content_type = this->fields[CONTENT_TYPE_KEY];
                    this_string_hash += this_content_type.front();
                    SharedArray<std::string> x_content_type = x.fields[CONTENT_TYPE_KEY];
                    x_string_hash += x_content_type.front();
                }
                return this_string_hash == x_string_hash;
            }
            bool operator!= (HttpRequest& x) {
                return false == (*this == x) ? true : false;
            }
    };

    class HttpResponse {
        public:
            std::string status_code;
            std::string version;
            std::unordered_map<std::string, std::string> fields;
            SharedArray<std::string> cookies;
            SharedArray<uint8_t> content;
            size_t content_length;

            HttpResponse(): version("HTTP/1.1"), status_code("500") {
    			fields["Connection"].append("close");
            }
            ~HttpResponse() = default;

            HttpResponse(ByteEStream * io) {
                ReaderTaskDisallow<SharedArray<uint8_t>> whitespace_chain;
                ReaderTaskDisallow<SharedArray<uint8_t>> whitespace_lc_chain;
                ReaderTaskLC<SharedArray<uint8_t>> lowercase_task;
                initializeByteStreamReaderTasks(whitespace_chain, whitespace_lc_chain, lowercase_task);

                cookies.remove(0, cookies.size());
                fields.clear();

                version = io->read(" ").removeBack().toString();
                SharedArray<uint8_t> status_code = io->read(" \n");
                if (status_code.back() == ' ') {
                    io->read("\n");
                }
                status_code = status_code.removeBack().toString();
      
                int field_idx = 0; 
                while (field_idx < HTTP_FIELD_MAX) {
                    std::string field_name = io->read(":\n", &whitespace_lc_chain).toString();
                    if (field_name[field_name.size()-1] == '\n') {
                        printf("FOUND EMPTY NEW LINE AFTER PARSING FIELDS\n");
                        break;
                    }
                    field_name = field_name.substr(0, field_name.size()-1);
      
                    loggerPrintf(LOGGER_DEBUG, "field_name: '%s'\n", field_name.c_str());
      
                    ReaderTaskChain<SharedArray<uint8_t>> * chain = &whitespace_chain;
                    if (FIELD_VALUES_TO_LOWER_CASE.contains(field_name.c_str())) {
                        chain = &whitespace_lc_chain;
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
                     
                            if (field_name == "set-cookie") {
                                cookies.append(field_value);
                            } else {
                                fields[field_name].append(field_value);
                            }
                            if (field_name == "content-length") {
                                content_length = atoi(field_value.c_str());
                            }
                        }
                    }
                }
                // TODO: read until EOF?
                if (content_length != SIZE_MAX) {
                    content = io->readEls((size_t)content_length);
                }
                if (field_idx == HTTP_FIELD_MAX) {
                    throw std::runtime_error("Too many fields in request.");
                }
            }

            std::string toString() {
                // header = this->version + " " + strconv.Itoa(status_code) + " " + status_map[r.StatusCode] + "\n";
                std::string response = this->version + " " + this->status_code + "\n";
                response += WylesLibs::format("Content-Length: {d}", content.size());
                for (const auto& [key, value] : this->fields) {
    				response += key + ": " + value + "\n";
    			}
                for (const auto& value : this->cookies) {
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

    class HttpProcessorItem {
        public:
            HttpRequest request;
            RequestProcessor * processor;
            SharedArray<RequestFilter> request_filters;
            SharedArray<ResponseFilter> response_filters;

            HttpProcessorItem() = default;
            HttpProcessorItem(HttpRequest request, RequestProcessor * processor, SharedArray<RequestFilter> request_filters, SharedArray<ResponseFilter> response_filters): 
                request(request), processor(processor), request_filters(request_filters), response_filters(response_filters) {}
            ~HttpProcessorItem() = default;

            bool operator== (HttpProcessorItem& x) {
                return this->request == x.request;
            }
    };

    extern SharedArray<HttpProcessorItem> requestMap;

// @ static

    // assuming amd64 - what year are we in? LMAO
    static_assert(sizeof(Url) == 
        sizeof(std::string) + 
        sizeof(std::unordered_map<std::string, std::string>)
    ); 
    static_assert(sizeof(Url) == 88);

    // usage:
    //  HTTP("/example", "", example, {}, {}) 
    //      "" value for content type matches all requests that match the path and method.
    //  HTTP("/example", "application/json", example, {request_filter}, {response_filter})
    //  HTTPHTTP("/example", example)
    #define HTTP(method, path, content_type, func, request_filters, response_filters) \
        static auto func ## _map = WylesLibs::Http::requestMap.uniqueAppend({HttpRequest(std::string(path), std::string(method), content_type), func, request_filters, response_filters});
    #define HTTPHTTP(method, path, func) \
        HTTP(method, path, "", func, {}, {})

    /*
        #define HTTP_GET(path, content_type, func, request_filters, response_filters) \
            static auto func ## _map = WylesLibs::Http::requestMap.uniqueAppend({HttpRequest(std::string(path), std::string("GET"), content_type), func, request_filters, response_filters});
        #define HTTP_GETGET(path, func) \
            HTTP_GET(path, "", func, {}, {})
 
        #define HTTP_POST_A(path, content_type, func, request_filters, response_filters) \
            static auto func ## _map = WylesLibs::Http::requestMap.uniqueAppend({HttpRequest(std::string(path), std::string("POST"), content_type), func, request_filters, response_filters});
        #define HTTP_POST_B(path, content_type, func, request_filters, response_filters) \
            HTTP_POST_A(path, "", func, {}, {})
 
        #define HTTP_CONNECT(path, content_type, func, request_filters, response_filters) \
            static auto func ## _map = WylesLibs::Http::requestMap.uniqueAppend({HttpRequest(std::string(path), std::string("POST"), content_type), func, request_filters, response_filters});
        #define HTTP_CONNECTCONNECT(path, content_type, func, request_filters, response_filters) \
            HTTP_CONNECT(path, "", func, {}, {})
 
        #define HTTP_DELETE(path, content_type, func, request_filters, response_filters) \
            static auto func ## _map = WylesLibs::Http::requestMap.uniqueAppend({HttpRequest(std::string(path), std::string("POST"), content_type), func, request_filters, response_filters});
        #define HTTP_DELETE_2(path, content_type, func, request_filters, response_filters) \
            HTTP_DELETE(path, "", func, {}, {})
    */

    // Note to self, be better about procrastination while at the same time remaining poignant - especially when there's not a lot to unpacks. space-time! zesty! spicy! me gusta estudiar en la biblioteca!
};

#endif