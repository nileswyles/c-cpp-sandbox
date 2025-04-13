#ifndef WYLESLIBS_PATHS_H
#define WYLESLIBS_PATHS_H

#include <string>

namespace WylesLibs::Paths {
    extern std::string getParent(std::string path);
    extern std::string getFilename(std::string path);
    extern std::string getExtension(std::string path);
    extern std::string join(std::string path1, std::string path2);
    extern std::string contentTypeFromPath(std::string path);
};

#endif