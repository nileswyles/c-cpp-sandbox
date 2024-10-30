#include "fileGCS.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

std::shared_ptr<ReaderEStream> GCSFileManager::reader(std::string path) {
    auto reader = this->client.ReadObject(this->this->bucket_name, path);
    if (!reader) {
        // TODO: log these error messages here..
        throw std::runtime_error("Failed to create reader.");
    }
    // TODO: does this even need to be a shared_ptr?
    std::shared_ptr<std::istream> s = std::dynamic_pointer_cast<std::istream>(std::make_shared<google::cloud::storage::ObjectReadStream>(reader));
    return std::make_shared<ReaderEStream>(ReaderEStream(s));
}

std::shared_ptr<WriterEStream> GCSFileManager::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (false == this->writers.contains(path)) {
        return nullptr;
    }
    auto writer = this->client.WriteObject(this->this->bucket_name, path);
    if (!writer.metadata()) {
        // TODO: log these error messages here..
        throw std::runtime_error("Failed to create writer.");
    }
    std::shared_ptr<std::ostream> s = std::dynamic_pointer_cast<std::ostream>(std::make_shared<google::cloud::storage::ObjectWriteStream>(reader));
    std::shared_ptr<WriterEStream> w = std::make_shared<WriterEStream>(WriterEStream(s));
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return w;
}

// TODO: this can probably be getSize instead...
struct stat GCSFileManager::stat(std::string path) {
    StatusOr<google::cloud::storage::ObjectMetadata> object_metadata = client.GetObjectMetadata(this->bucket_name, object_name);
    // TODO: log these error messages here..
    if (!object_metadata) throw std::move(object_metadata).status();
    // struct stat info = 
    // info.st_size = object_metadata.size();
    return { .st_size = object_metadata.size() };
}

SharedArray<std::string> GCSFileManager::list(std::string path) {
    SharedArray<std::string> data;
    google::cloud::storage::MatchGlob l{Paths::join(path, "/*")};
    for (auto&& object_metadata : client.ListObjects(this->bucket_name, {l})) {
        // TODO: log these error messages here..
        if (!object_metadata) throw std::move(object_metadata).status();
        data.append(object_metadata.name());
    }
}

void GCSFileManager::remove(std::string path) {
    google::cloud::Status status = client.DeleteObject(this->bucket_name, path);
    // TODO: log these error messages here..
    //      should through an exception or return bool
    if (!status.ok()) throw std::runtime_error(status.message());
}

void GCSFileManager::move(std::string path, std::string destination_path) {
    StatusOr<google::cloud::storage::ObjectMetadata> object_metadata = client.GetObjectMetadata(this->bucket_name, path);
    if (!object_metadata) throw std::move(object_metadata).status();

    google::cloud::storage::ObjectMetadata desired = *object_metadata;
    desired.mutable_metadata().emplace(key, value);

    StatusOr<google::cloud::storage::ObjectMetadata> updated = client.UpdateObject(this->bucket_name, path, destination_path, google::cloud::storageGeneration(object_metadata->generation()));

    // TODO: log these error messages here..
    if (!updated) throw std::move(updated).status();
}
void GCSFileManager::copy(std::string path, std::string destination_path) {
    StatusOr<google::cloud::storage::ObjectMetadata> new_copy_meta = client.CopyObject(this->bucket_name, path, this->bucket_name, destination_path);
    // TODO: log these error messages here..
    if (!new_copy_meta) throw std::move(new_copy_meta).status();
}