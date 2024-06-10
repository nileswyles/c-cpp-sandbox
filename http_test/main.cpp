#include "web/http/http.h"
#include "web/http/connection.h"
#include "web/http/config.h"
#include "web/services.h"
#include "web/server_context.h"

#include "controllers/example.h"

#ifndef LOGGER_HTTP_SERVER_TEST
#define LOGGER_HTTP_SERVER_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP_SERVER_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Http;

class WebsocketJsonRpcConnection: public ConnectionUpgrader {
    public:
        WebsocketJsonRpcConnection(std::string path, std::string protocol): ConnectionUpgrader(path, protocol) {}

        // this makes more sense extension of some Connection class?
        //  alright, I know I "c@n'T gr@MM@R" but definetly not that bad...
        uint8_t onConnection(int conn_fd) {
            printf("Established websocket connection...\n");

            return 1;
        }
};

void requestFilterMiddleware(HttpRequest * request) {
    // for login filters, initializing auth context, etc
}

void responseFilterMiddleware(HttpResponse * response) {}

HttpResponse * requestDispatcher(HttpRequest * request) {
    // map
    //   resource_path -> JsonObject, request_handler(JsonObject)...
    //
    // request -> 

    // okay, so that works for application/json, but for other content?
    //  so maybe,

    //  map
    //     resource_path -> Array<uint8_t>, request_handler<Array<uint8_t>>
    //  contentType == 

    // alright so, http connection class is responsible for:
    //      calling the resource handler...
    //          question is selection...
    //              - resource path
    //              - content type
    //              or ideally a combination of both?

    //          can this also be defined by caller?
    //              definitely, yes programatically (passing "function" to connection class)
    //              definitely, yes declaratively (passing "config" to connection class)

    //              which is more work for the implementer? 
    //                  let's explore both options...
    //                      programatically:
    //                          one function that can then do whatever....
    //  
    //                      declaratively:
    //                          This requires some planning... maybe defining how it will be used...

    //                          config == representation of the following table
    //                              table:
    //                                  resource_path, content_type, resource_handler 

    //                                  resource_path, content_type, resource_handler, arg_type // (too strong lol)

    //                                  resource_path, content_type, (ResourceObjectHandler) // (too strong lol)

    //                                  ResourceObjectHandler
    //                                      type  // (too strong lol)

    //                                  ResourceObjectHandlerJson
    //                                      type  // (too strong lol)

    //                                  ... 

    //                                  ResourceObject
    //                                      resource_path
    //                                      content_type
    //                                      ResourceObjectHandler
    //                                  
    //                          so more generally, 
    //                              this is matter of "serializing" (pseudo-serializing) 
    //                              (strongly typed to semi-weakly typed (intermediate) to very-weakly typed).. parsing but not fully...

    //                          LMAO...
    //                          what is a string really?
    //                          
    //                          Okay, so let's think about performance implications of this abstraction/generalization... 
    //                              Space/time complexity is 

    //                          constant iteration is draining!!!!!
    //                          
    //                          but there's probably value in obfuscating... the mystery in the magic...?

    //                          hmm... fuck it... lol... thinking about this too much... it really depends...

    //                  building that datastructure is probably just as much work as programatically so let's not go the declarative route for this one!
    //                      can I generalize and just go straight to this for things in CPP (strongly typed, no reflection?)

    //                       - to scared to jump off that ledge... does that make me weak? lmao, the irony...
    //                  
    //                  lol... blehhhhhhhhhhhhh...... derppppppppp....

    // what a balancing act
    requestFilterMiddleware(request);
    HttpResponse * response = nullptr;
    if ("/example" == request->url.path && "application/json" == request->fields["content-type"][0]) {
        response = Controller::example(request);
    } else if ("/example2" == request->url.path && "multipart/byteranges" == request->fields["content-type"][0]) {
        response = Controller::example2(request);
    } else if ("/example3" == request->url.path) {
        response = Controller::example3(request);
    }
    responseFilterMiddleware(response);

    return response;
}

// Generally, direct access to global contexts (state) are frowned upon... but this should be fine...
//    alternatively, can move this to each service layer module...
//    or pass along 'only' via function params... (absolutely not, some centralization is needed to keep this from becoming a mess.) 

// But again, it might depend on the application? This is an example application to illustrate how you would use the http library...
static ServerContext * server_context = nullptr;

// again, why?
extern ServerContext * WylesLibs::getServerContext() {
    // LOL
    if (server_context == nullptr) {
        std::string msg = "ServerContext is a null pointer.";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    } else {
        return server_context;
    }
}

static HttpConnection connection;

static uint8_t connectionHandler(int conn_fd) {
    // because instance functions can't be passed to function pointer args?
    return connection.onConnection(conn_fd);
}

int main(int argc, char * argv[]) {
    int ret = 0;
    try {
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Launching HTTP Server.\n");
        HttpServerConfig config("config.json");
        
        ServerContext context(config);
        server_context = &context;

        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Created config object.\n");
        Array<ConnectionUpgrader *> upgraders;
        WebsocketJsonRpcConnection upgrader("/testpath", "jsonrpc");
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Created upgrader object.\n");
        // WebsocketCastProtobufConnection upgrader("/testpath", "cast-protobuf"); LOL
        // WebsocketCastJsonRpcConnection upgrader("/testpath", "cast-jsonrpc"); LOL
    
        // WebsocketCastProtobufConnection upgrader("/test-cast-protobuf"); LOL
        // WebsocketCastJsonRpcConnection upgrader("/test-cast-jsonrpc"); LOL
    
        // hmmmmm... if only this weren't so................
        // ConnectionUpgrader * lame = &upgrader;
        // upgraders.append(lame);
        // upgraders.append((ConnectionUpgrader *)&upgrader);
        // lol...
    
        // alright, still a shit ton of open questions but this should work?
        connection = HttpConnection(config, requestDispatcher, upgraders); 
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Created connection object.\n");
        serverListen(config.address.c_str(), (uint16_t)config.port, connectionHandler);
    } catch (const std::exception& e) {
        // redundant try/catch? let's show where exception handled...
        loggerPrintf(LOGGER_ERROR, "%s\n", e.what());
        // exit program same way as if this weren't caught...
        throw e;
    }
    return ret;
}