#include "file/file_gcs.h"
#include "paths.h"

using namespace WylesLibs;
using namespace WylesLibs::File;
namespace gcs = ::google::cloud::storage;

/uint64_t GCSFileManager::stat(std::string path) {
    google::cloud::StatusOr<gcs::ObjectMetadata> object_metadata = client.GetObjectMetadata(this->bucket_name, path);
    if (!object_metadata) throw std::move(object_metadata).status();
    return static_cast<uint64_t>(object_metadata->size());
}

SharedArray<std::string> GCSFileManager::list(std::string path) {
    SharedArray<std::string> data;
    for (auto&& object_metadata : client.ListObjects(this->bucket_name, gcs::MatchGlob(Paths::join(path, "/*")))) {
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
    google::cloud::StatusOr<gcs::ObjectMetadata> object_metadata = client.GetObjectMetadata(this->bucket_name, path);
    if (!object_metadata) throw std::move(object_metadata).status();

    gcs::ObjectMetadata desired = *object_metadata;

    desired.set_name(destination_path);

    google::cloud::StatusOr<gcs::ObjectMetadata> updated = client.UpdateObject(this->bucket_name, path, desired, gcs::Generation(object_metadata->generation()));

    // TODO: log these error messages here..
    if (!updated) throw std::move(updated).status();
}
void GCSFileManager::copy(std::string path, std::string destination_path) {
    google::cloud::StatusOr<gcs::ObjectMetadata> new_copy_meta = client.CopyObject(this->bucket_name, path, this->bucket_name, destination_path);
    // TODO: log these error messages here..
    if (!new_copy_meta) throw std::move(new_copy_meta).status();
}