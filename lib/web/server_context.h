#ifndef WYLESLIBS_SERVER_CONTEXT_H
#define WYLESLIBS_SERVER_CONTEXT_H

#include "server_config.h"
#include "key_generator.h"

#include "file/file.h"

using namespace WylesLibs::File;

namespace WylesLibs {

class ServerContext {
    public:
        ServerConfig config;
        UniqueKeyGenerator key_generator;
        ESharedPtr<FileManager> file_manager;

        ServerContext(ServerConfig config): ServerContext(config, ESharedPtr<FileManager>(new FileManager)) {}
        ServerContext(ServerConfig config, ESharedPtr<FileManager> file_manager): config(config), file_manager(file_manager), 
            key_generator(UniqueKeyGenerator(config, UniqueKeyGeneratorStore(file_manager, Paths::join(config.resources_root, "sequence_store")))) {}
        ~ServerContext() = default;
};

extern ServerContext * getServerContext();

}
#endif 