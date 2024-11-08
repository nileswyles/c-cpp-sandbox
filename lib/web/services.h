#ifndef WYLESLIB_SERVER_SERVICES_H
#define WYLESLIB_SERVER_SERVICES_H

#include "server_context.h"
#include "parser/multipart/multipart_file.h"

namespace Service {

static MultipartFile createMultipartFile(std::string file_name) {
    return MultipartFile(getServerContext(), file_name);
}

}

#endif