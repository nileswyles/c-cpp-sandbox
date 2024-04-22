#ifndef WYLESLIBS_PATHS_H
#define WYLESLIBS_PATHS_H

#include <string>
using namespace std;

// Assuming unix paths for now... binary size not a concern, so definition in .h
namespace WylesLibs::Paths {

static string getParent(string path) {
    size_t pos = path.find_last_of("/");
    return pos == string::npos ? path : path.substr(0, pos);
}

static string getFilename(string path) {
    size_t pos = path.find_last_of("/");
    string name_w_ext = pos == string::npos ? path : path.substr(pos);

    size_t endpos = path.find_last_of(".");
    return endpos == string::npos ? name_w_ext : path.substr(pos, endpos-pos);
}

static string getExtension(string path) {
    size_t pos = path.find_last_of(".");
    return pos == string::npos ? path : path.substr(pos);
}

}

#endif WYLESLIBS_PATHS_H