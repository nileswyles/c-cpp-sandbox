#include "tester.h"

#include <stdexcept>

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#ifndef LOGGER_HTTP_SERVER_TEST
#define LOGGER_HTTP_SERVER_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP_SERVER_TEST
#include "logger.h"

using namespace WylesLibs;

using namespace WylesLibs::Test;

int connect() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        throw std::runtime_error("error creating socket.");
    } else {
        struct in_addr addr; 
        if (inet_aton("127.0.0.1", &addr) == 0) {
            throw std::runtime_error("Error parsing address string, 127.0.0.1");
        } else {
            struct sockaddr_in a = {
                AF_INET,
                ntohs(8080),
                addr,
            };
            if (connect(fd, (struct sockaddr *)(&a), sizeof(struct sockaddr_in)) == -1) {
                throw std::runtime_error("error binding." + errno);
            } else {
                socklen_t timeval_len = sizeof(struct timeval);
                struct timeval timeout = {
                    .tv_sec = 44,
                    .tv_usec = 0,
                };
                setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeval_len);
                setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, timeval_len);
   
                struct timeval rcv_timeout = {0};
                getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, &timeval_len);
                struct timeval snd_timeout = {0};
                getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &snd_timeout, &timeval_len);
                loggerPrintf(LOGGER_DEBUG, "SOCK OPTS: SO_RCVTIMEO %lds %ldus, SO_SNDTIMEO %lds %ldus\n", rcv_timeout.tv_sec, rcv_timeout.tv_usec, snd_timeout.tv_sec, snd_timeout.tv_usec);

                return fd;
            }
        }
    }
}

void testHttpServer(TestArg * t) {
    std::string request("GET / HTTP/1.1\n");
    request += "User-Agent: PostmanRuntime/7.39.0\n";
    request += "Accept: */*\n";
    request += "Postman-Token: 9b100f3a-9d44-4bbe-96b1-b5939705b0be\n";
    request += "Host: localhost:8080\n";
    request += "Accept-Encoding: gzip, deflate, br\n";
    request += "Connection: keep-alive\n\n";

    int fd = connect();

    write(fd, request.c_str(), request.size());

    uint8_t buf[1024];
    int ret = read(fd, buf, 1024);

    if (ret > 0) {
        buf[ret] = 0;
        std::string response((const char *)buf); 
        loggerPrintf(LOGGER_TEST, "Actual Response (%ld): \n%s\n\n", response.size(), response.c_str());

        std::string expected_response("HTTP/1.1 200\n");
        expected_response += "Content-Type: text/html\n";
        expected_response += "Content-Length: 162\n";
        expected_response += "Connection: close\n\n";
        expected_response += "<!doctype html>\n";
        expected_response += "<html>\n";
        expected_response += "  <head>\n";
        expected_response += "    <title>This is the title of the webpage!</title>\n";
        expected_response += "  </head>\n";
        expected_response += "  <body>\n";
        expected_response += "    <p>This is an example paragraph.</p>\n";
        expected_response += "  </body>\n";
        expected_response += "</html>";

        loggerPrintf(LOGGER_TEST, "Expected Response (%ld): \n%s\n", expected_response.size(), expected_response.c_str());
        if (response == expected_response) {
            t->fail = false;
        }
    }
    close(fd);
}

void testHttpServerSocketTimeout(TestArg * t) {
    std::string request("GET / HTTP 1.1\n");
    request += "b";
    int fd = connect();

    write(fd, request.c_str(), request.size());

    uint8_t buf[1024];
    int ret = read(fd, buf, 1024);
    loggerPrintf(LOGGER_DEBUG, "read ret: %d, errno: %u\n", ret, errno);
    if (ret > 0) {
        buf[ret] = 0;
        std::string response((const char *)buf); 
        loggerPrintf(LOGGER_TEST, "Actual Response (%ld): \n%s\n\n", response.size(), response.c_str());

        std::string expected_response("HTTP/1.1 500\n");
        expected_response += "Connection: close\n\n";
        loggerPrintf(LOGGER_TEST, "Expected Response (%ld): \n%s\n", expected_response.size(), expected_response.c_str());
        if (response == expected_response) {
            t->fail = false;
        }
    }
    close(fd);
}

int main(int argc, char * argv[]) {
    Tester t;

    t.addTest(testHttpServer);
    t.addTest(testHttpServerSocketTimeout);

    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        t.run(argv[1]);
    } else {
        t.run(nullptr);
    }

    return 0;
}