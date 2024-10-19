#ifndef WYLESLIBS_MULTIPART_FILE_H
#define WYLESLIBS_MULTIPART_FILE_H

#include <stdexcept>
#include <string>
#include <map>

#include "web/server_context.h"
#include "paths.h"
#include "key_generator.h"

namespace WylesLibs {

class MultipartFile {
    private:
        std::string resource_root;
    public:
        std::string id;
        std::string name;

        MultipartFile(): resource_root("./") {}
        MultipartFile(std::string name) {
            resource_root = "";
            id = "";
            name = name;
        }
        MultipartFile(ServerContext * context, std::string name) {
            resource_root = context->config.resources_root;
            id = context->key_generator.next();
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