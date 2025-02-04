#ifndef WYLESLIBS_MULTIPART_FILE_H
#define WYLESLIBS_MULTIPART_FILE_H

#include <stdexcept>
#include <string>
#include <map>

#include "paths.h"

namespace WylesLibs {

class MultipartFile {
    private:
        std::string resource_root;
    public:
        std::string id;
        std::string name;

        MultipartFile(): resource_root("./") {}
        MultipartFile(std::string name) {
            id = "";
            resource_root = "";
            name = name;
        }
        MultipartFile(std::string id, std::string resource_root, std::string name) {
            id = id;
            resource_root = resource_root;
            name = name;
        }
        ~MultipartFile() = default;

        std::string getResourcePath() {
            return Paths::join(this->resource_root, this->id);
        }
        bool operator== (MultipartFile& other) {
            // assume resource root is a server-wide thing...
            return this->name == other.name;
        }
};
}
#endif