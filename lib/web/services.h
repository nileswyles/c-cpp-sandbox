#ifndef WYLESLIB_SERVER_SERVICES_H
#define WYLESLIB_SERVER_SERVICES_H

#include "web/server.h"
#include "parser/multipart/multipart_file.h"

namespace Service {
    static MultipartFile createMultipartFile(std::string file_name) {
        Server * context = getServerContext();
        std::string resource_root = context->config.resources_root;
        std::string id = context->key_generator.next();
        return MultipartFile(id, resource_root, file_name);
    }
};

#endif