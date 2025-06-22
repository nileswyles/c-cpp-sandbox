#ifndef WYLESLIBS_VALIDATION_H
#define WYLESLIBS_VALIDATION_H

#include <stdlib.h>
#include <string>
#include <regex>

namespace WylesLibs {

    extern bool isValidIPAddress(std::string address) {
        std::string octet_str;
        bool is_valid = true;
        for (size_t i = 0; i < address.size(); i++) {
            if (address.at(i) == '.') {
                int octet = atoi(octet_str.c_str());
                if (octet < 0 || octet > 255) {
                    is_valid = false;
                    break;
                }
            } else {
                octet_str += address.at(i);
            }
        }
        return is_valid;
    }

    extern bool isValidBTAddress(std::string address) {
        std::regex r("[a-fA-F0-9]{2}\:?[a-fA-F0-9]{2}\:?[a-fA-F0-9]{2}\:?[a-fA-F0-9]{2}\:?[a-fA-F0-9]{2}\:?[a-fA-F0-9]{2}");
        return std::regex_search(address, r);
    }
};

#endif