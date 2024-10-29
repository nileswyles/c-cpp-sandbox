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

SharedArray<uint8_t> FileManager::read(std::string path, size_t offset, size_t size) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Unable to read file at: " + path);
    }
    IOStream r(fd);
    if (size == SIZE_MAX) {
        struct stat stat_info = {};
        int lol = fstat(fd, &stat_info);
        size = stat_info.st_size - offset;
    }
    SharedArray<uint8_t> file = r.readBytes(size);
    close(fd);
    return file;
}

void FileManager::write(std::string path, SharedArray<uint8_t> buffer, size_t offset) {
    // open every time a problem?
    std::fstream s{path, s.binary | s.out};
    if (!s.is_open()) {
        throw std::runtime_error("Unable to open file at: " + path);
    } else {
        s.seekp(offset);
        s.write((const char *)buffer.start(), buffer.size()); // binary output
        s.flush();
        s.close();
    }

}
void FileManager::write(std::string path, SharedArray<uint8_t> buffer, bool append) {
    WylesLibs::File::write(path, buffer, append);
}

FileStat FileManager::stat(std::string path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Unable to read file at: " + path);
    }
    struct stat stat_info = {};
    int lol = fstat(fd, &stat_info);

    FileStat stat;
    stat.path = path;
    stat.size = stat_info.st_size;
    close(fd);

    return stat;
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