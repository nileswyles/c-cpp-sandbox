#include "fileS3.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

SharedArray<uint8_t> S3FileManager::read(std::string path, size_t offset, size_t size) {
}

void S3FileManager::write(std::string path, SharedArray<uint8_t> buffer, size_t offset) {
}
void S3FileManager::write(std::string path, SharedArray<uint8_t> buffer, bool append) {
}

struct stat S3FileManager::stat(std::string path) {
}
SharedArray<std::string> S3FileManager::list(std::string path) {
}

void S3FileManager::remove(std::string path) {
}
void S3FileManager::move(std::string path, std::string destination_path) {
}
void S3FileManager::copy(std::string path, std::string destination_path) {
}