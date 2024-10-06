#ifndef WYLESLIBS_AUTHORIZATION_H
#define WYLESLIBS_AUTHORIZATION_H

#include <string>

namespace WylesLibs {

class Authorization {
    std::string token;
    uint64_t user;
    uint64_t group;
    public:
        Authorization() {}
        Authorization(std::string token): token(token) {}
        ~Authorization() = default;
};

}

#endif