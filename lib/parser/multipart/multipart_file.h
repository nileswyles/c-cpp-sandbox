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

        MultipartFile(ServerContext * context, std::string name) {
            resource_root = context->config.resources_root;
            id = context->key_generator.next();
            name = name;
        }

        std::string getResourcePath() {
            return Paths::join(this->resource_root, this->id);
        }
};
}
#endif