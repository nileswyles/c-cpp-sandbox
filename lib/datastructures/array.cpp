#include "datastructures/array.h"

using namespace WylesLibs;

template<>
void WylesLibs::addElement<const char *>(const char ** buffer, const size_t pos, const char * el) {
    char * new_cstring = newCArray<char>(strlen(el) + 1);
    strcpy(new_cstring, el);
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "String copied: %p, '%s'\n", el, el);
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "New String: %p, '%s'\n", new_cstring, new_cstring);
    buffer[pos] = new_cstring;
}

// const char value...
// T == const char *
// T * == const char **
// T ** == const char ***
template<>
void WylesLibs::deleteCArray<const char *>(const char ** e_buf, size_t size) {
    loggerPrintf(LOGGER_DEBUG, "Deleting C Array of type 'const char *' of size: %lu\n", size);
    if (e_buf != nullptr) {
        for (size_t i = 0; i < size; i++) {
            // deletes string. see allocation in addElement function above...
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "String being deleted: '%s'\n", e_buf[i]);
            if (e_buf[i] != nullptr) {
                delete[] e_buf[i];
            }
        }
        // deletes array of string pointers
        delete[] e_buf;
    }
}

template<>
void WylesLibs::deleteCArrayElement<const char *>(const char ** buffer, size_t pos) {
    loggerPrintf(LOGGER_DEBUG, "Deleting element of ptr type 'const char *' at %lu\n", pos);
    // deletes string. see allocation in addElement function above...
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "String being deleted: '%s'\n", buffer[pos]);
    if (buffer != nullptr && buffer[pos] != nullptr) {
        delete[] buffer[pos];
    }
}

template<>
ssize_t WylesLibs::arrayFind<const char *>(const char ** e_buf, size_t size, const char * el) {
    for (size_t i = 0; i < size; i++) {
        if (strcmp(e_buf[i], el) == 0) {
            return i;
        }
    }
    return -1;
}

template<>
int WylesLibs::nlognsortCompare<const char *>(ArraySort sortOrder, const char * A, const char * B) {
    int ret = strcmp(A, B);
    if (sortOrder == ARRAY_SORT_DESCENDING) {
        ret *= -1;
    }
    return ret;
}

// This can be further dried
template<>
std::string WylesLibs::arrayToString<char *>(char ** e_buf, size_t size) {
    std::string ret;
    for (size_t i = 0; i < size; i++) {
        if (e_buf[i] != nullptr) {
            ret += e_buf[i];
            printf("%s\n", e_buf[i]);
            if (i < size - 1) {
                ret += " ";
            }
        }
    }
    return ret;
}

template<>
std::string WylesLibs::arrayToString<std::string>(std::string * e_buf, size_t size) {
    std::string ret;
    for (size_t i = 0; i < size; i++) {
        ret += e_buf[i];
        if (i < size - 1) {
            ret += " ";
        }
    }
    return ret;
}