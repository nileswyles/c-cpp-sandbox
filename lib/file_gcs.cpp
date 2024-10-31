#include "file_gcs.h"
#include "paths.h"

using namespace WylesLibs;
using namespace WylesLibs::File;
namespace gcs = ::google::cloud::storage;

// ! IMPORTANT - It's important the user understands how the ReadObject functionality works.
//               Normally (size == SIZE_MAX), the ReadObject call returns a stream representing a file on GCS. 
//               Read's are buffered, the entire file is not held in memory and apparently seek functionality is non-existent. 
//
//               This method provides a way of getting a stream representing a range of bytes of a file on GCS.
//               With trade-off of slightly more overhead, you are provided control of chunk sizes and "better" seeking functionality.

//               TODO: think about whether it should be it's own function called ranges_reader or something?
std::shared_ptr<ReaderEStream> GCSFileManager::reader(std::string path, size_t offset, size_t size) {
    if (this->this_shared == nullptr) {
        this->this_shared = std::make_shared<FileManager>(dynamic_cast<FileManager>(this));
    }
    std::shared_ptr<ReaderEStream> stream;
    gcs::ObjectReadStream reader;
    // TODO: can I read from offset to end of file? I think so. 
    // TODO: inclusive?
    reader = this->client.ReadObject(this->bucket_name, path, gcs::ReadRange(static_cast<std::int64_t>(offset), static_cast<std::int64_t>(offset + size)));
    if (!reader) {
        // TODO: log these error messages here..
        throw std::runtime_error("Failed to create range reader.");
    }
    // TODO: cast then shared or shared then cast?
    stream = std::make_shared<ReaderEStream>(this->this_shared, path, offset, size, 
                                             std::dynamic_pointer_cast<std::basic_istream<char>>(
                                                std::make_shared<gcs::ObjectReadStream>(std::move(reader))
                                            )
             );
    return stream;
}

std::shared_ptr<std::basic_ostream<char>> GCSFileManager::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (false == this->writers.contains(path)) {
        return nullptr;
    }
    auto writer = this->client.WriteObject(this->bucket_name, path);
    if (!writer.metadata()) {
        // TODO: log these error messages here..
        throw std::runtime_error("Failed to create writer.");
    }
    std::shared_ptr<std::basic_ostream<char>> w = std::make_shared<std::basic_ostream<char>>(
                                                      std::dynamic_pointer_cast<std::ostream>(
                                                          std::make_shared<google::cloud::storage::ObjectWriteStream>(std::move(writer))
                                                      )
                                                  );
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return w;
}

// TODO: this can probably be getSize instead...
uint64_t GCSFileManager::stat(std::string path) {
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