#include "file_gcs.h"
#include "paths.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

std::shared_ptr<ReaderEStream> GCSFileManager::reader(std::string path) {
    auto reader = this->client.ReadObject(this->bucket_name, path);
    if (!reader) {
        // TODO: log these error messages here..
        throw std::runtime_error("Failed to create reader.");
    }
    // TODO: cast then shared or shared then cast?
    std::shared_ptr<std::basic_istream<char>> s = std::dynamic_pointer_cast<std::basic_istream<char>>(std::make_shared<google::cloud::storage::ObjectReadStream>(std::move(reader)));
    return std::make_shared<ReaderEStream>(s);
}

std::shared_ptr<WriterEStream> GCSFileManager::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (false == this->writers.contains(path)) {
        return nullptr;
    }
    auto writer = this->client.WriteObject(this->bucket_name, path);
    if (!writer.metadata()) {
        // TODO: log these error messages here..
        throw std::runtime_error("Failed to create writer.");
    }
    std::shared_ptr<std::ostream> s = std::dynamic_pointer_cast<std::ostream>(std::make_shared<google::cloud::storage::ObjectWriteStream>(std::move(writer)));
    std::shared_ptr<WriterEStream> w = std::make_shared<WriterEStream>(s);
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return w;
}

// TODO: this can probably be getSize instead...
uint64_t GCSFileManager::stat(std::string path) {
    google::cloud::StatusOr<google::cloud::storage::ObjectMetadata> object_metadata = client.GetObjectMetadata(this->bucket_name, path);
    if (!object_metadata) throw std::move(object_metadata).status();
    return static_cast<uint64_t>(object_metadata->size());
}

SharedArray<std::string> GCSFileManager::list(std::string path) {
    namespace gcs = ::google::cloud::storage;
    SharedArray<std::string> data;
    for (auto&& object_metadata : client.ListObjects(this->bucket_name, gcs::Prefix(path))) {
        // TODO: log these error messages here..
        if (!object_metadata) throw std::move(object_metadata).status();
        data.append(object_metadata->name());
    }
    return data;
}

void GCSFileManager::remove(std::string path) {
    google::cloud::Status status = client.DeleteObject(this->bucket_name, path);
    // TODO: log these error messages here..
    //      should through an exception or return bool
    if (!status.ok()) throw std::runtime_error(status.message());
}

void GCSFileManager::move(std::string path, std::string destination_path) {
    namespace gcs = ::google::cloud::storage;
    google::cloud::StatusOr<gcs::ObjectMetadata> object_metadata = client.GetObjectMetadata(this->bucket_name, path);
    if (!object_metadata) throw std::move(object_metadata).status();

    gcs::ObjectMetadata desired = *object_metadata;

    desired.set_name(destination_path);

    google::cloud::StatusOr<gcs::ObjectMetadata> updated = client.UpdateObject(this->bucket_name, path, desired, gcs::Generation(object_metadata->generation()));

    // TODO: log these error messages here..
    if (!updated) throw std::move(updated).status();
}
void GCSFileManager::copy(std::string path, std::string destination_path) {
    google::cloud::StatusOr<google::cloud::storage::ObjectMetadata> new_copy_meta = client.CopyObject(this->bucket_name, path, this->bucket_name, destination_path);
    // TODO: log these error messages here..
    if (!new_copy_meta) throw std::move(new_copy_meta).status();
}