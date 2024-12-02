#include "file/stream_factory.h"
#include "paths.h"

#include <fstream>

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_STREAM_FACTORY
#define LOGGER_LEVEL_STREAM_FACTORY GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_STREAM_FACTORY
#define LOGGER_STREAM_FACTORY 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_STREAM_FACTORY

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_STREAM_FACTORY
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

ESharedPtr<std::basic_istream<char>> StreamFactory::reader(std::string path, size_t offset, size_t size) {
    std::basic_ifstream<char> * ifstream = new std::basic_ifstream<char>(path);
    if (false == ifstream->is_open()) {
        std::string msg = "Unable to open file at path: " + path + " for reading.\n";
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    ifstream->seekg(offset);
    return ESharedPtr<std::basic_istream<char>>(
        dynamic_cast<std::basic_istream<char> *>(
            ifstream
        )
    );
}

ESharedPtr<std::basic_ostream<char>> StreamFactory::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (true == this->writers.contains(path)) {
        // TODO: maybe just return writer again...
        // return nullptr;
    }
    std::basic_ofstream<char> * ofstream = new std::basic_ofstream<char>(path, std::fstream::binary | std::fstream::out);
    if (false == ofstream->is_open()) {
        std::string msg = "Unable to open file at path: " + path + " for writing.\n";
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return ESharedPtr<std::basic_ostream<char>>(
        dynamic_cast<std::basic_ostream<char> *>(
            ofstream
        )
    );
}

// TODO: store stream ptrs, up-cast flush and close?
//      or implement my own streams that implements close... gcs doesn't support close anyways? but will this be an issue for other solutions?
//      not as extensible?
void StreamFactory::removeWriter(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
//      for now let's assume destructors flush and close appropriately...
    this->writers.erase(path);
    pthread_mutex_unlock(&this->writers_lock);
}