#include "tester.h"

#include <stdexcept>

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <string.h>

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
    std::string request("GET / HTTP/1.1\n");
    request += "b";
    int fd = connect();

    write(fd, request.c_str(), request.size());

    struct timespec ts_before;
    clock_gettime(CLOCK_MONOTONIC, &ts_before);

    uint8_t buf[1024];
    int ret = read(fd, buf, 1024);

    struct timespec ts_after;
    clock_gettime(CLOCK_MONOTONIC, &ts_after);
    loggerPrintf(LOGGER_DEBUG, "read ret: %d, errno: %u\n", ret, errno);

    if (ret > 0) {
        buf[ret] = 0;
        std::string response((const char *)buf); 
        loggerPrintf(LOGGER_TEST, "Actual Response (%ld): \n%s\n\n", response.size(), response.c_str());

        std::string expected_response("HTTP/1.1 500\n");
        expected_response += "Connection: close\n\n";
        loggerPrintf(LOGGER_TEST, "Expected Response (%ld): \n%s\n", expected_response.size(), expected_response.c_str());

        uint64_t delta = ts_after.tv_sec - ts_before.tv_sec;
        loggerPrintf(LOGGER_TEST, "Time Before: %lu, Time After: %lu, Time Delta: %lu\n", ts_before.tv_sec, ts_after.tv_sec, delta);
        if (response == expected_response && delta < 4 && delta > 1) {
            t->fail = false;
        }
    }
    close(fd);
}

static uint32_t getTimeout(std::string timeout) {
    std::string socket_timeout_request("GET /timeout/");
    socket_timeout_request += timeout;
    socket_timeout_request += " HTTP/1.1\n\n";

    int fd = connect();
    write(fd, socket_timeout_request.c_str(), socket_timeout_request.size());

    char buf[1024];
    int ret = read(fd, buf, 1024);
    if (ret > 0) {
        buf[ret] = 0;
        std::string resp(buf);
        size_t split_i = resp.find_first_of(":");
        size_t split_j = resp.find_first_of("}");
        
        std::string timeout = resp.substr(split_i, split_j - split_i);

        return atoi(timeout.c_str());
    }
    close(fd);

    return -1;
}

static void setSocketTimeout(uint32_t value) {
    char timeout[11];
    sprintf(timeout, "%d", value);
    
    char content_length[11];
    sprintf(content_length, "%ld", strlen(timeout) + 12);
    std::string socket_timeout_request("POST /timeout HTTP/1.1\n");
    socket_timeout_request += "Content-Type: application/json\n";
    socket_timeout_request += "Content-Length: ";
    socket_timeout_request += content_length;
    socket_timeout_request += "\n\n";
    socket_timeout_request += "{\"socket\": ";
    socket_timeout_request += timeout;
    socket_timeout_request += "}";

    int fd = connect();
    write(fd, socket_timeout_request.c_str(), socket_timeout_request.size());
    close(fd);
}

void testHttpServerConnectionTimeout(TestArg * t) {
    uint32_t initial_socket_timeout = getTimeout("socket");
    uint32_t connection_timeout = getTimeout("connection");
    setSocketTimeout(connection_timeout + 2); // set socket timeout greater than connection.

    std::string request("GET / HTTP/1.1\n");
    request += "b";
    int fd = connect();
    write(fd, request.c_str(), request.size());
    struct timespec ts_before;
    clock_gettime(CLOCK_MONOTONIC, &ts_before);

    uint8_t buf[1024];
    int ret = read(fd, buf, 1024);

    struct timespec ts_after;
    clock_gettime(CLOCK_MONOTONIC, &ts_after);
    loggerPrintf(LOGGER_DEBUG, "read ret: %d, errno: %u\n", ret, errno);
    if (ret > 0) {
        buf[ret] = 0;
        std::string response((const char *)buf); 
        loggerPrintf(LOGGER_TEST, "Actual Response (%ld): \n%s\n\n", response.size(), response.c_str());

        std::string expected_response("HTTP/1.1 500\n");
        expected_response += "Connection: close\n\n";
        loggerPrintf(LOGGER_TEST, "Expected Response (%ld): \n%s\n", expected_response.size(), expected_response.c_str());

        uint64_t delta = ts_after.tv_sec - ts_before.tv_sec;
        loggerPrintf(LOGGER_TEST, "Time Before: %lu, Time After: %lu, Time Delta: %lu\n", ts_before.tv_sec, ts_after.tv_sec, delta);
        if (response == expected_response && delta < 17 && delta > 14) {
            t->fail = false;
        }
    }
    close(fd);

    setSocketTimeout(initial_socket_timeout);
}

int main(int argc, char * argv[]) {
    Tester t;

    t.addTest(testHttpServer);
    t.addTest(testHttpServerSocketTimeout);
    t.addTest(testHttpServerConnectionTimeout);

    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        t.run(argv[1]);
    } else {
        t.run(nullptr);
    }

    return 0;
}