#include "paths.h"

using namespace std;

string SueroLibs::Paths::getParent(string path) {
    size_t pos = path.find_last_of("/");
    return pos == string::npos ? path : path.substr(0, pos);
}

string SueroLibs::Paths::getFilename(string path) {
    size_t pos = path.find_last_of("/");
    string name_w_ext = pos == string::npos ? path : path.substr(pos);

    size_t endpos = path.find_last_of(".");
    return endpos == string::npos ? name_w_ext : path.substr(pos, endpos-pos);
}

string SueroLibs::Paths::getExtension(string path) {
    size_t pos = path.find_last_of(".");
    return pos == string::npos ? path : path.substr(pos);
}