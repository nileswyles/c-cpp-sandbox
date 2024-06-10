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

static Array<RequestFilter> requestFilters{};
static Array<ResponseFilter> responseFilters{};
// hmm... unordered maps here?
static map<std::string, map<std::string, RequestProcessor *>> requestMap{
    {"/example", {{"application/json", Controller::example }}},
    {"/example2", {{"multipart/byteranges", Controller::example2 }}},
    {"/example3", {{"", Controller::example3 }}}
};

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
        connection = HttpConnection(config, requestMap, requestFilters, responseFilters, upgraders); 
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