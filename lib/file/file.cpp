#include "file/file.h"

#include <filesystem>

using namespace WylesLibs;
using namespace WylesLibs::File;

uint64_t FileManager::stat(std::string path) {
    // TODO: throw exception or return UINT64_MAX or 0?
    return std::filesystem::file_size(path);
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