#ifndef LOGGER_HTTP_EXAMPLE_CONTROLLER
#define LOGGER_HTTP_EXAMPLE_CONTROLLER 1
#endif

#include "controllers/example.h"

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP_EXAMPLE_CONTROLLER
#include "logger.h"

using namespace Controller;

HTTP_METHOD("/example", example)
extern HttpResponse * Controller::example(HttpRequest * request) {
    HttpResponse * response = new HttpResponse;
    response->status_code = "200";

    return response;
}

HTTP_METHOD("/example2", example2)
extern HttpResponse * Controller::example2(HttpRequest * request) {
    HttpResponse * response = new HttpResponse;
    response->status_code = "200";

    return response;
}

HTTP_METHOD("/example3", example3)
extern HttpResponse * Controller::example3(HttpRequest * request) {
    HttpResponse * response = new HttpResponse;
    response->status_code = "200";

    return response;
}