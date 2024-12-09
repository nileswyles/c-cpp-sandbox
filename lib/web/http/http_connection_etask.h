#ifndef WYLESLIBS_HTTP_CONNECTION_ETASK_H
#define WYLESLIBS_HTTP_CONNECTION_ETASK_H  

#include "web/server_connection_etask.h"
#include "web/http/http_types.h"
#include "web/http/http.h"
#include "parser/json/json.h"
#include "estream/byteestream.h"

using namespace WylesLibs::Http;

namespace WylesLibs {
    class HttpConnectionETask: public ServerConnectionETask {
        private:
            HttpServer * server;

            void parseRequest(HttpRequest * request, ByteEStream * reader);
            void processRequest(ByteEStream * io, HttpRequest * request);
            HttpResponse * handleStaticRequest(HttpRequest * request);
            bool handleWebsocketRequest(ByteEStream * io, HttpRequest * request);
#ifdef WYLESLIBS_HTTP_DEBUG
            HttpResponse * handleTimeoutRequests(ByteEStream * io, HttpRequest * request);
#endif
            HttpResponse * requestDispatcher(HttpRequest * request);
            void writeResponse(HttpResponse * response, ByteEStream * io);

        public:
            HttpConnectionETask(int fd, uint64_t socket_timeout_s, HttpServer * c): server(c), ServerConnectionETask(fd, socket_timeout_s) {}
            HttpConnectionETask(int fd, uint64_t socket_timeout_s): ServerConnectionETask(fd, socket_timeout_s) {
                Server * s = getServerContext();
                server = dynamic_cast<HttpServer *>(s);
            }
            ~HttpConnectionETask() override = default;
            void run() override;
            void onExit() override;
    };
};
#endif