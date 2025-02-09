#ifndef WYLESLIBS_CMDER_H
#define WYLESLIBS_CMDER_H

#include "datastructures/array.h"

#include <string>

namespace WylesLibs {
    extern std::string esystem(const char * cmd, SharedArray<char *> args);
};
#endif