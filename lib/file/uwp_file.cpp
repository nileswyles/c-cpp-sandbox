#include "file/file.h"

#include <ppltasks.h>
#include <synchapi.h>

using namespace Windows::Storage;

using namespace WylesLibs;
using namespace WylesLibs::File;

#define OPERATION_TIMEOUT_MILLIS 2000
        
bool FileManager::exists(std::string path) {
    return std::filesystem::exists(path);
}

uint64_t FileManager::stat(std::string path) {
    return std::filesystem::file_size(path);
}

SharedArray<std::string> FileManager::list(std::string path) {
    // ! IMPORTANT - this ensures a consistent api, and if you really want Async, then wrap around another task on invocation?
    SharedArray<std::string> list;

    StorageFolder^ folder;
    // TODO: std::string to String
    bool sync = true;
    create_task(StorageFolder::GetFolderFromPathAsync(root))
    .then([&sync, &folder](StorageFolder^ f){
        folder = f;
        String^ output = f->Name;
        // hmm.. this is interesting...
        OutputDebugString(output->Begin());
        sync = false;
    });

    uint16_t time_elapsed = 0;
    while(sync && time_elapsed < OPERATION_TIMEOUT_MILLIS) {
        Sleep(1000);
        time_elapsed += sleep_amount;
    }

    sync = true;
    // Get a read-only vector of the file objects
    // and pass it to the continuation.
    create_task(folder->GetFilesAsync())        
       // outputString is captured by value, which creates a copy
       // of the shared_ptr and increments its reference count.
    .then ([list, &sync] (IVectorView\<StorageFile^>^ files) {        
        for (size_t i = 0 ; i < files->Size; i++) {
           list.append(files->GetAt(i)->Name->Data());
        }
        sync = false;
    });

    // TODO: or task.wait();

    time_elapsed = 0;
    while(sync && time_elapsed < OPERATION_TIMEOUT_MILLIS) {
        Sleep(1000);
        time_elapsed += sleep_amount;
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