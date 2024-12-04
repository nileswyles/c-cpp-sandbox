#include "web/http/http.h"
#include "web/http/http_types.h"
#include "web/http/connection.h"
#include "web/http/config.h"
#include "web/http/http_connection_etask.h"
#include "web/services.h"
#include "web/server_context.h"

#include "controllers/example.h"

#include "file/file_watcher.h"
#ifdef WYLESLIBS_GCS_BUILD
#include "file/file_gcs.h"
#endif

#include <pthread.h>

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
        uint8_t onConnection(ByteEStream * io) {
            printf("Established websocket connection...\n");
            printf("MESSAGE FROM CLIENT: %s\n", io->read("}").toString().c_str());
            return 1;
        }
};

static SharedArray<RequestFilter> requestFilters{};
static SharedArray<ResponseFilter> responseFilters{};
// TODO:
// hmm... unordered maps here?
// and yes you're all crazy this should and was working?
//  maybe move the initialization of this to constructor of each controller class?
//      something like Controller classes initialized in main function and receive reference to this map...
//      and constructor populates... worse because controller initialization still needed? also this? ("because instance functions can't be passed to function pointer args?")

//      alternatively,
//      define a map in each controller file and main function gets and merges before intializing http server?
//      maybe that pattern is only required on larger projects?
static map<std::string, map<std::string, RequestProcessor *>> requestMap{
    {"/example", {{"application/json", Controller::example }}},
    {"/example2", {{"multipart/byteranges", Controller::example2 }}},
    {"/example3", {{"application/x-www-form-urlencoded", Controller::example3 }}},
    {"/example4", {{"multipart/formdata", Controller::example }}},
    {"/exampleDontCare", {{"", Controller::example }}} 
};

static ServerContext * server_context = nullptr;

extern ServerContext * WylesLibs::getServerContext() {
    if (server_context == nullptr) {
        std::string msg = "ServerContext is a null pointer.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    } else {
        return server_context;
    }
}

static HttpServer connection;

// ! IMPORTANT - this pattern must be implemented in all multithreaded applications that leverage the etasker stuff.
//                  this is decoupled from etasker to support multiple etasker instances, other ways of threading, process forking, etc.. (see fileWatcher)
#include "multithreaded_signals.h"
void * sig_handler(int sig, siginfo_t * info, void * context) {
    pthread_t pthread = pthread_self();
    if (thread_specific_sig_handlers.contains(pthread)) {
        (thread_specific_sig_handlers[pthread])(sig, info, context);
    } else {
        raise(sig);
    }
}

void initProcessSigHandler() {
    struct sigaction act = { 0 };
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &sig_handler;
    if (sigaction(SIGKILL, &act, NULL) == -1 || sigaction(SIGSEGV, &act, NULL) == -1) {
        std::string msg("Failed to configure sig action and sig handler.\n");
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}

int main(int argc, char * argv[]) {
    int ret = 0;
    try {
        initProcessSigHandler();
        // ! IMPORTANT
        // Might be good to get in the habit of evaluating size of types used from libs to determine whether to malloc or not?
        //  modern stack sizes are large enough but might matter in an embedded system?
        //  can also just look at header file...
        //  you can't assume they ptr everything and there's actually a compelling argument to not.
        //  More generally, larger stack size allocations vs larger heap.

        //  so, if need access to more memory you can call new where needed at point of creation of each thread. Like here.
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Size of HttpServer: %lu, Size of ServerContext: %lu\n", sizeof(HttpServer), sizeof(ServerContext));
        // if you see this in header files, then maybe their worthy lol...
        // static_assert(sizeof(HttpServer) == 0); 
        // but maybe this isn't a good idea because now we definetly have this type in program?
        // static_assert(sizeof(Array<uint8_t>) == 0); 

        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Launching HTTP Server.\n");
        HttpServerConfig config("config.json");
        // ServerContext context(config, std::dynamic_pointer_cast<FileManager>(ESharedPtr<GCSFileManager>(ESharedPtr<GCSFileManager>(std::make_shared<GCSFileManager>("test-bucket-free-tier")))));
        ServerContext context(config);
        server_context = &context;

        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Created config object.\n");
        SharedArray<ConnectionUpgrader *> upgraders;
        WebsocketJsonRpcConnection upgrader("/testpath", "jsonrpc");
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Created upgrader object.\n");
        ConnectionUpgrader * upgrader_ptr = &upgrader;
        upgraders.append(upgrader_ptr);
        // upgraders.append(&upgrader_ptr, 1);
    
        fileWatcherThreadStart();

        connection = HttpServer(config, requestMap, requestFilters, responseFilters, upgraders, context.file_manager); 
        connection.initialize();

        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Created connection object.\n");
        connection.listen(config.address.c_str(), (uint16_t)config.port);
    } catch (const std::exception& e) {
        // loggerPrintf(LOGGER_INFO, "%s\n", std::stacktrace::current().to_string().c_str());
        // redundant try/catch? let's show where exception handled...
        loggerPrintf(LOGGER_INFO, "%s\n", e.what());
        // exit program same way as if this weren't caught...
        throw e;
    }
    return ret;
}