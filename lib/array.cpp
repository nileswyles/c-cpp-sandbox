#include "array.h"

using namespace WylesLibs;

// ?
// template<typename T>
// void addElement(T ** e_buf, const size_t pos, T el) {
//     (*e_buf)[pos] = el;
//     loggerPrintf(LOGGER_DEBUG, "NEW BUFFER ELEMENT: [%x], OLD BUFFER ELEMENT: [%x]\n", (*e_buf)[pos], el);
// }

template<>
void WylesLibs::addElement<const char *>(const char ** buffer, const size_t pos, const char * el) {
    char * new_cstring = newCArray<char>(strlen(el) + 1);
    strcpy(new_cstring, el);
    loggerPrintf(LOGGER_DEBUG, "String copied: %p, %s\n", el, el);
    loggerPrintf(LOGGER_DEBUG, "New String: %p, %s\n", new_cstring, new_cstring);
    buffer[pos] = new_cstring;
}

// template<typename T>
// void deleteCArray(T ** e_buf, size_t size) {
//     delete[] *e_buf;
//     delete e_buf;
// }
// template<>
// void WylesLibs::deleteCArray<void *>(void *** e_buf, size_t size) {
//     loggerPrintf(LOGGER_DEBUG, "Deleting C Array of type 'void *'\n");
//     for (size_t i = 0; i < size; i++) {
//         delete[] (*e_buf)[i];
//     }
//     delete[] *e_buf;
//     delete e_buf;
// }
// const char value...
// T == const char *
// T * == const char **
// T ** == const char ***
template<>
void WylesLibs::deleteCArray<const char *>(const char *** e_buf, size_t size) {
    loggerPrintf(LOGGER_DEBUG, "Deleting C Array of type 'const char *'\n");
    for (size_t i = 0; i < size; i++) {
        // TODO:
        // delete[] *e_buf[i];
        // delete[] *e_buf + i;
    }
    delete[] *e_buf;
    delete e_buf;
}
template<>
void WylesLibs::deleteCArrayElement<const char *>(const char ** e_buf, size_t pos) {
    loggerPrintf(LOGGER_DEBUG, "Deleting element of ptr type 'const char *'\n");

    // TODO:
    // delete[] e_buf[pos];
    // delete[] (e_buf + pos);
}
// template<>
// void WylesLibs::deleteCArrayElement<void *>(void ** e_buf, size_t pos) {
//     loggerPrintf(LOGGER_DEBUG, "Deleting element of ptr type 'void *'\n");
//     delete e_buf[pos];
// }

template<>
ssize_t WylesLibs::arrayFind<const char *>(const char *** e_buf, size_t size, const char * el) {
    for (size_t i = 0; i < size; i++) {
        if (strcmp((*e_buf)[i], el) == 0) {
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