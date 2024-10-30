#include "fileS3.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

GCSReader GCSFileManager::getReader(std::string uuid) {
    pthread_mutex_lock(&this->reader_lock);
    GCSReader r;
    if (false == this->reader_map.contains(uuid)) {
        this->reader_map[uuid] = GCSReader(reader);
    }
    r = this->reader_map[uuid];
    pthread_mutex_unlock(&this->reader_lock);
    return r;
}

GCSWriter GCSFileManager::getWriter(std::string uuid) {
    pthread_mutex_lock(&this->writer_lock);
    GCSWriter w;
    if (false == this->writer_map.contains(uuid)) {
        auto writer = this->client.WriteObject(this->bucket_name, uuid);
        if (!writer.metadata()) {
          std::cerr << "Error creating object: " << writer.metadata().status()
              << "\n";
          return 1;
        }
        this->writer_map[path] = GCSWriter(writer);
    }
    w = this->writer_map[path];
    pthread_mutex_unlock(&this->writer_lock);
    return w;
}

// TODO: pass array by reference?
//  yeah, might need to make unique key not just path. writes, just one at a time?
std::shared_ptr<std::istream> GCSFileManager::read(std::string path) {
    auto reader = this->client.ReadObject(this->bucket_name, path);
    if (!reader) {
      std::cerr << "Error reading object: " << reader.status() << "\n";
      return 1;
    }
    std::shared_ptr<std::istream> stream = std::make_shared<std::istream>(reader);
    pthread_mutex_lock(&this->reader_lock); 
    // LMAO? that's annoying... implement a Queue? I think reader_lock around read function is effectively the same thing?
    //  This seems like a familiar problem... 
    //    we want to prevent deleting while another thread has lock (is mid read)...
    //    can probably remove per reader lock too..

    //  uuid might be better solution? then only need per reader lock?

    GCSReader r = getReader(path);
    // pthread_mutex_lock(&r.lock);
    // TODO: yeah, that was the initial apprehension - hopefully it doesn't keep entire file in memory... 
    r.reader.seekg(offset);
    SharedArray<uint8_t> buffer{&r.reader, size};
    // pthread_mutex_lock(&this->reader_lock);
    if (r.bytes_read + size == r.reader.size()) {
        r.reader.close();
        this->reader_map.erase(path);
    } else {
        this->reader_map[path].bytes_read += size;
    }
    pthread_mutex_unlock(&this->reader_lock);
    // pthread_mutex_unlock(&r.lock);
    return buffer;
}

void GCSFileManager::write(std::string path, SharedArray<uint8_t> buffer, size_t offset) {
    pthread_mutex_lock(&this->writer_lock);
    GCSWriter w = getWriter(path);
    // pthread_mutex_lock(&w.lock);
    // TODO: yeah, that was the initial apprehension - hopefully this functionality works...
    w.writer.seekp(offset);

    w.writer.write(buffer.start(), buffer.size());
    w.writer.flush();
    w.writer.close();
    // pthread_mutex_unlock(&w.lock);

    // pthread_mutex_lock(&this->writer_lock);
    this->writer_map.erase(path);
    pthread_mutex_unlock(&this->writer_lock);
}
void GCSFileManager::write(std::string path, SharedArray<uint8_t> buffer, bool append) {
    pthread_mutex_lock(&this->writer_lock);
    GCSWriter w = getWriter(path);
    // pthread_mutex_lock(&w.lock);
    // TODO: update SharedArray to support that operator...
    if (false == append) {
        w.writer.seekp(0)
    }
    w.writer.write(buffer.start(), buffer.size());
    w.writer.flush();
    w.writer.close();
    // pthread_mutex_unlock(&w.lock);

    // pthread_mutex_lock(&this->writer_lock);
    this->writer_map.erase(path);
    pthread_mutex_unlock(&this->writer_lock);
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