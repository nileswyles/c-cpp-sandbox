#include "file.h"

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

std::shared_ptr<std::istream> FileManager::reader(std::string path) {
    return std::dynamic_pointer_cast<std::istream>(std::make_shared<std::ifstream>(path));
}

std::shared_ptr<std::ostream> FileManager::writer(std::string path) {
    return std::dynamic_pointer_cast<std::ostream>(std::make_shared<std::ofstream>(path, std::fstream::binary | std::fstream::out));
}

void FileManager::removeWriter(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    this->writers.erase(path);
    pthread_mutex_unlock(&this->writers_lock);
}

struct stat FileManager::stat(std::string path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Unable to read file at: " + path);
    }
    struct stat stat_info = {};
    int lol = fstat(fd, &stat_info);
    close(fd);

    return stat_info;
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