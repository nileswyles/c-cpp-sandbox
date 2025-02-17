#include "web/http/http.h"
#include "web/http/http_types.h"
#include "web/http/connection.h"
#include "web/http/config.h"
#include "web/http/http_connection_etask.h"
#include "web/services.h"

#include "controllers/example.h"

#include "file/file_watcher.h"
#ifdef WYLESLIBS_GCS_BUILD
#include "file/file_gcs.h"
#else
#include "file/file.h"
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
            printf("MESSAGE FROM CLIENT: %s\n", io->read<SharedArray<uint8_t>>("}").toString().c_str());
            return 1;
        }
};

static SharedArray<RequestFilter> requestFilters{};
static SharedArray<ResponseFilter> responseFilters{};

static HttpServer server_context;
extern Server * WylesLibs::getServerContext() {
    return dynamic_cast<Server *>(&server_context);
}

// ! IMPORTANT - this pattern must be implemented in all multithreaded applications that leverage the etasker stuff.
void sig_handler(int sig, siginfo_t * info, void * context) {
    pthread_t pthread = pthread_self();
    if (sig == SIGTERM) {
        if (ETasker::thread_specific_sig_handlers.contains(pthread)) {
            ETasker * etasker = ETasker::thread_specific_sig_handlers[pthread];
            etasker->threadSigAction(sig, info, context);
        }
    }
}

void sig_handler1(int sig) {
    pthread_t pthread = pthread_self();
    if (sig == SIGTERM) {
        if (ETasker::thread_specific_sig_handlers.contains(pthread)) {
            ETasker * etasker = ETasker::thread_specific_sig_handlers[pthread];
            etasker->threadSigAction(sig, nullptr, nullptr);
        }
    }
}

void initProcessSigHandler() {
    // struct sigaction act = { 0 };
    // act.sa_flags = SA_SIGINFO;
    // act.sa_sigaction = &sig_handler;
    // if (sigaction(SIGKILL, &act, NULL) == -1 || sigaction(SIGSEGV, &act, NULL) == -1) {
    //     std::string msg("Failed to configure sig action and sig handler.\n");
    //     loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
    //     throw std::runtime_error(msg);
    // }

    // workaround because idk?
    signal(SIGTERM, sig_handler1);
    signal(SIGSEGV, SIG_IGN);
}

int main(int argc, char * argv[]) {
    int exit_value = 0;
    try {
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Launching HTTP Server.\n");
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Size of HttpServer: %lu\n", sizeof(HttpServer));

        initProcessSigHandler();

        HttpServerConfig config("config.json");
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Size of HttpServerConfig: %lu\n", sizeof(HttpServerConfig));

        SharedArray<ConnectionUpgrader *> upgraders;
        WebsocketJsonRpcConnection upgrader("/testpath", "jsonrpc");

        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Created upgrader object.\n");
        ConnectionUpgrader * upgrader_ptr = &upgrader;
        upgraders.append(upgrader_ptr);
        fileWatcherThreadStart();

        ESharedPtr<FileManager> file_manager(new FileManager);
        // ESharedPtr<FileManager> file_manager = ESharedPtr<GCSFileManager>(new GCSFileManager("test-bucket-free-tier"));
        server_context = HttpServer(config, WylesLibs::Http::requestMap, requestFilters, responseFilters, upgraders, file_manager); 
        server_context.listen(config.address.c_str(), (uint16_t)config.port);
    } catch (const std::exception& e) {
        loggerPrintf(LOGGER_INFO, "%s\n", e.what());
        exit_value = 1;
    }
    return exit_value;
}