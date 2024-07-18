#ifndef EXAMPLE_CONTROLLER_H
#define EXAMPLE_CONTROLLER_H

#include "web/http/http.h"

using namespace WylesLibs::Http;

namespace Controller {

extern HttpResponse * example(HttpRequest * request);
extern HttpResponse * example2(HttpRequest * request);
extern HttpResponse * example3(HttpRequest * request);

}

#endif