#ifndef LOGGER_HTTP_EXAMPLE_CONTROLLER
#define LOGGER_HTTP_EXAMPLE_CONTROLLER 1
#endif

#include "controllers/example.h"

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP_EXAMPLE_CONTROLLER
#include "logger.h"

using namespace Controller;

// stilll don't understand why Controller:: is needed... but okay
extern HttpResponse * Controller::example(HttpRequest * request) {
    HttpResponse * response = new HttpResponse;
    response->status_code = "200";

    return response;
}

extern HttpResponse * Controller::example2(HttpRequest * request) {
    HttpResponse * response = new HttpResponse;
    response->status_code = "200";

    return response;
}

extern HttpResponse * Controller::example3(HttpRequest * request) {
    HttpResponse * response = new HttpResponse;
    response->status_code = "200";

    return response;
}