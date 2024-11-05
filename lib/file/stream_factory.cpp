#include "file/stream_factory.h"
#include "logger.h"
#include "paths.h"

#include <fstream>
using namespace WylesLibs;
using namespace WylesLibs::File;

std::shared_ptr<std::basic_istream<char>> StreamFactory::reader(std::string path, size_t offset, size_t size) {
    std::shared_ptr<std::basic_ifstream<char>> ifstream = std::make_shared<std::ifstream>(path);
    if (false == ifstream->is_open()) {
        std::string msg = "Unable to open file at path: " + path + " for reading.\n";
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    ifstream->seekg(offset);
    return std::dynamic_pointer_cast<std::basic_istream<char>>(ifstream);
}

std::shared_ptr<std::basic_ostream<char>> StreamFactory::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (true == this->writers.contains(path)) {
        return nullptr;
    }
    std::shared_ptr<std::basic_ofstream<char>> ofstream = std::make_shared<std::basic_ofstream<char>>(path, std::fstream::binary | std::fstream::out);
    if (false == ofstream->is_open()) {
        std::string msg = "Unable to open file at path: " + path + " for writing.\n";
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return std::dynamic_pointer_cast<std::basic_ostream<char>>(ofstream);
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