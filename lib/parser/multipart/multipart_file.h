#ifndef WYLESLIBS_MULTIPART_FILE_H
#define WYLESLIBS_MULTIPART_FILE_H

#include <stdexcept>
#include <string>
#include <map>

#include "server_config.h"
#include "paths.h"

namespace WylesLibs {

class MultipartFile {
    private:
        std::string resource_root;
    public:
        std::string id;
        std::string name;

        MultipartFile(): resource_root("./") {
            throw std::runtime_error("What even are you doing!");
        }

        // TODO:
        //  this serverconfig will likely become servercontext with access to db/service layer...
        MultipartFile(ServerConfig config, std::string name) {
            resource_root = config.resources_root;
            // TODO: generate ID, for now, we'll just overwrite file...
            // id = 
            name = name;
        }

        std::string getResourcePath() {
            return Paths::join(this->resource_root, this->id);
        }
};
}
#endif