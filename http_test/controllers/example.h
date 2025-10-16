#ifndef EXAMPLE_CONTROLLER_H
#define EXAMPLE_CONTROLLER_H

#include "http_types.h"

using namespace WylesLibs::Http;

namespace Controller {

extern HttpResponse * example(HttpRequest * request);
extern HttpResponse * example2(HttpRequest * request);
extern HttpResponse * example3(HttpRequest * request);

};

#endif