#include "file.h"

#include <filesystem>
#include <filesystem>

// TODO: I think this might be better in .h file because polymorphism but let's see...
#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

std::shared_ptr<ReaderEStream> FileManager::reader(std::string path) {
    std::shared_ptr<std::basic_istream<char>> s = std::dynamic_pointer_cast<std::basic_istream<char>>(std::make_shared<std::ifstream>(path));
    return std::make_shared<ReaderEStream>(s);
}

std::shared_ptr<WriterEStream> FileManager::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (false == this->writers.contains(path)) {
        return nullptr;
    }
    std::shared_ptr<std::ostream> s = std::dynamic_pointer_cast<std::ostream>(std::make_shared<std::ofstream>(path, std::fstream::binary | std::fstream::out));
    std::shared_ptr<WriterEStream> w = std::make_shared<WriterEStream>(s);
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return w;
}

// TODO: store stream ptrs, up-cast flush and close?
//      or implement my own streams that implements close... gcs doesn't support close anyways? but will this be an issue for other solutions?
//      not as extensible?
void FileManager::removeWriter(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
//      for now let's assume destructors flush and close appropriately...
    this->writers.erase(path);
    pthread_mutex_unlock(&this->writers_lock);
}

uint64_t FileManager::stat(std::string path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Unable to read file at: " + path);
    }
    struct stat stat_info = {};
    int lol = fstat(fd, &stat_info);
    close(fd);

    return static_cast<uint64_t>(stat_info.st_size);
}
SharedArray<std::string> FileManager::list(std::string path) {
    // if not directory (or is file)
    //  throw exception
    //  else directory listing.

    // TODO: might seem inefficient but range of domain is limited to begin with.
    SharedArray<std::string> list;
    for (auto const& dir_entry: std::filesystem::directory_iterator{path}) {
        list.append(dir_entry.path().string());
    }
    if (list.size() == 0) {
        throw std::runtime_error("This must be a file.");
    }
    return list;
}

void FileManager::remove(std::string path) {
    std::filesystem::path fs_path{path};

    // ddd
    // if (std::filesystem::is_directory(fs_path)) {
        std::filesystem::remove_all(fs_path);
    // } else {
        // std::filesystem::remove(fs_path);
    // }
}
void FileManager::move(std::string path, std::string destination_path) {
    std::filesystem::rename(std::filesystem::path{path}, std::filesystem::path{destination_path});
}
void FileManager::copy(std::string path, std::string destination_path) {
    std::filesystem::copy(std::filesystem::path{path}, std::filesystem::path{destination_path});
}