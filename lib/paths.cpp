#include "paths.h"

using namespace std;
using namespace WylesLibs::Paths;

extern string WylesLibs::Paths::getParent(string path) {
    size_t pos = path.find_last_of("/");
    return pos == string::npos ? path : path.substr(0, pos);
}

extern string WylesLibs::Paths::getFilename(string path) {
    size_t pos = path.find_last_of("/");
    string name_w_ext = pos == string::npos ? path : path.substr(pos);

    size_t endpos = path.find_last_of(".");
    return endpos == string::npos ? name_w_ext : path.substr(pos, endpos-pos);
}

extern string WylesLibs::Paths::getExtension(string path) {
    size_t pos = path.find_last_of(".");
    return pos == string::npos ? path : path.substr(pos);
}

extern string WylesLibs::Paths::join(string path1, string path2) {
    // ./blah/blah/ /path2/file2.html - ./blah/blah/path2/file/file.html
    // ../blah/../ path2/file2.html - ../blah/../path2/file2.html

    // TODO: must have at least one element.
    if (path1.size() > 0 && path1[path1.size() - 1] != '/') {
        path1 += "/";
    }
    if (path2.size() > 0 && path2[0] == '/') {
        path2 = path2.substr(1);
    }
    return path1 + path2;
}

extern string WylesLibs::Paths::contentTypeFromPath(string path) {
    string contentType;
    string ext = getExtension(path);
    if (ext == ".html") {
       contentType = "text/html";
    } else if (ext == ".js") {
       contentType = "text/javascript";
    } else if (ext == ".css") {
       contentType = "text/css";
    } else {
       contentType = "none";
    }
    return contentType;
}