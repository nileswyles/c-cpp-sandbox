#ifndef WYLESLIBS_SERVER_CONTEXT_H
#define WYLESLIBS_SERVER_CONTEXT_H

#include "server_config.h"
#include "key_generator.h"

namespace WylesLibs {

class ServerContext {
    public:
        ServerConfig config;
        UniqueKeyGenerator key_generator;
        ServerContext(ServerConfig config): config(config), 
            key_generator(UniqueKeyGenerator(config, UniqueKeyGeneratorStore(Paths::join(config.resources_root, "sequence_store")))) {}
};

extern ServerContext * getServerContext();

}
#endif 