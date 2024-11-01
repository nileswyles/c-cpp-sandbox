#include "file/stream_factory.h"
#include "paths.h"

#include <fstream>
using namespace WylesLibs;
using namespace WylesLibs::File;

std::shared_ptr<std::basic_istream<char>> StreamFactory::reader(std::string path, size_t offset, size_t size) {
    std::shared_ptr<std::basic_ifstream<char>> ifstream = std::make_shared<std::ifstream>(path);
    ifstream->seekg(offset);
    return std::dynamic_pointer_cast<std::basic_istream<char>>(ifstream);
}

std::shared_ptr<std::basic_ostream<char>> StreamFactory::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (false == this->writers.contains(path)) {
        return nullptr;
    }
    std::shared_ptr<std::basic_ostream<char>> w = std::dynamic_pointer_cast<std::basic_ostream<char>>(
                                                      std::make_shared<std::ofstream>(path, std::fstream::binary | std::fstream::out)
                                                  );
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return w;
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