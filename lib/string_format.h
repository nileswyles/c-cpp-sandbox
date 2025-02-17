#ifndef WYLESLIBS_STRING_FORMAT_H
#define WYLESLIBS_STRING_FORMAT_H

#include <string>

namespace WylesLibs {
#define WYLESLIBS_STRING_FORMAT_DEFAULT_ARG_TYPE_CHAR '|'
class Arg {
    public:
        void * ptr;
        std::string expanded_value;
        char type;
        Arg(): ptr(nullptr), type(WYLESLIBS_STRING_FORMAT_DEFAULT_ARG_TYPE_CHAR), expanded_value("") {}
        Arg(void * ptr, char type, std::string expanded_value): ptr(ptr), type(type), expanded_value(expanded_value) {}
        virtual ~Arg() {}
};

extern std::string format(std::string format, ...);
#define MAX_FORMATTED_STRING_SIZE 1024
extern std::string ssprintf(std::string format, ...);
};
#endif