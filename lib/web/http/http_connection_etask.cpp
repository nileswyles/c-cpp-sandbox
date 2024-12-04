#include "web/http/http_connection_etask.h"

#include "web/http/http.h"
#include "estream/byteestream.h"

void HttpConnectionETask::run() {
    ByteEStream eio;
#ifdef WYLESLIBS_SSL_ENABLED
    SSLEStream sslio;
#endif
    HttpRequest request;
    ByteEStream * io;
    bool acceptedTLS = false;
    try {
#ifdef WYLESLIBS_SSL_ENABLED
        if (true == this->connection->config.tls_enabled) {
            sslio = SSLEStream(this->connection->context, this->fd, this->connection->config.client_auth_enabled); // initializes ssl object for connection
            acceptedTLS = true;
            io = dynamic_cast<ByteEStream *>(&sslio);
        } else {
            eio = ByteEStream(this->fd);
            io = &eio;
        }
#else
        eio = ByteEStream(this->fd);
        io = &eio;
#endif
        this->connection->parseRequest(&request, io);
        loggerPrintf(LOGGER_DEBUG, "Request path: '%s', method: '%s'\n", request.url.path.c_str(), request.method.c_str());
        this->connection->processRequest(io, &request);
    } catch (const std::exception& e) {
        loggerPrintf(LOGGER_INFO, "Exception thrown while processing request: %s\n", e.what());
        if (true == this->connection->config.tls_enabled && false == acceptedTLS) {
            // then deduce error occured while initializing ssl
            loggerPrintf(LOGGER_DEBUG, "Error accepting and configuring TLS connection.\n");
        } else {
            // respond with empty HTTP status code 500
            HttpResponse * err = new HttpResponse;
            writeResponse(err, io);
        }
    }
}

void HttpConnectionETask::onExit() {
    close(this->fd);
}