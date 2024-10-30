#include "fileGCS.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

std::shared_ptr<std::istream> GCSFileManager::reader(std::string path) {
    auto reader = this->client.ReadObject(this->bucket_name, path);
    if (!reader) {
        throw std::runtime_error("Failed to create reader.");
    }
    return std::dynamic_pointer_cast<std::istream>(std::make_shared<google::cloud::storage::ObjectReadStream>(reader));
}

std::shared_ptr<std::ostream> GCSFileManager::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (false == this->writers.contains(path)) {
        // if empty ostream try again indefinetly?
        return std::make_shared<std::ostream>();
    }
    auto writer = this->client.WriteObject(this->bucket_name, path);
    if (!writer.metadata()) {
        throw std::runtime_error("Failed to create writer.");
    }
    std::shared_ptr<std::ostream> w = std::dynamic_pointer_cast<std::ostream>(std::make_shared<google::cloud::storage::ObjectWriteStream>(reader));
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return w;
}

struct stat GCSFileManager::stat(std::string path) {
      StatusOr<gcs::ObjectMetadata> object_metadata =
        client.GetObjectMetadata(bucket_name, object_name);
    if (!object_metadata) throw std::move(object_metadata).status();

    std::cout << "The metadata for object " << object_metadata->name()
              << " in bucket " << object_metadata->bucket() << " is "
              << *object_metadata << "\n";
    this->client.
}
SharedArray<std::string> GCSFileManager::list(std::string path) {
      for (auto&& object_metadata : client.ListObjects(bucket_name)) {
      if (!object_metadata) throw std::move(object_metadata).status();

      std::cout << "bucket_name=" << object_metadata->bucket()
                << ", object_name=" << object_metadata->name() << "\n";
    }
}

void GCSFileManager::remove(std::string path) {
      google::cloud::Status status =
        client.DeleteObject(bucket_name, object_name);

    if (!status.ok()) throw std::runtime_error(status.message());
    std::cout << "Deleted " << object_name << " in bucket " << bucket_name
              << "\n";
}
void GCSFileManager::move(std::string path, std::string destination_path) {
     StatusOr<gcs::ObjectMetadata> object_metadata =
        client.GetObjectMetadata(bucket_name, object_name);
    if (!object_metadata) throw std::move(object_metadata).status();

    gcs::ObjectMetadata desired = *object_metadata;
    desired.mutable_metadata().emplace(key, value);

    StatusOr<gcs::ObjectMetadata> updated =
        client.UpdateObject(bucket_name, object_name, desired,
                            gcs::Generation(object_metadata->generation()));

    if (!updated) throw std::move(updated).status();
    std::cout << "Object updated. The full metadata after the update is: "
              << *updated << "\n";
}
void GCSFileManager::copy(std::string path, std::string destination_path) {
      StatusOr<gcs::ObjectMetadata> new_copy_meta =
        client.CopyObject(source_bucket_name, source_object_name,
                          destination_bucket_name, destination_object_name);
    if (!new_copy_meta) throw std::move(new_copy_meta).status();

    std::cout << "Successfully copied " << source_object_name << " in bucket "
              << source_bucket_name << " to bucket " << new_copy_meta->bucket()
              << " with name " << new_copy_meta->name()
              << ".\nThe full metadata after the copy is: " << *new_copy_meta
              << "\n";
}