#include "datastructures/array.h"

using namespace WylesLibs;

template<>
void WylesLibs::addElement<const char *>(const char ** buf, const size_t pos, const char * el) {
    char * new_cstring = newCArray<char>(strlen(el) + 1);
    strcpy(new_cstring, el);
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "String copied: %p, '%s'\n", el, el);
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "New String: %p, '%s'\n", new_cstring, new_cstring);
    buf[pos] = new_cstring;
}

// const char value...
// T == const char *
// T * == const char **
// T ** == const char ***
template<>
void WylesLibs::deleteCArray<const char *>(const char *** e_buf, size_t size) {
    loggerPrintf(LOGGER_DEBUG, "Deleting C Array of type 'const char *' of size: %lu\n", size);
    if (e_buf != nullptr) {
        if (*e_buf != nullptr) {
            for (size_t i = 0; i < size; i++) {
                // deletes string. see allocation in addElement function above...
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "String being deleted: '%s'\n", (*e_buf)[i]);
                if (*e_buf[i] != nullptr) {
                    delete[] (*e_buf)[i];
                }
            }
            // deletes array of string pointers
            delete[] *e_buf;
        }
        // deletes container (pointer to array deleted above) 
        delete e_buf;
    }
}

template<>
void WylesLibs::deleteCArrayElement<const char *>(const char ** buf, size_t pos) {
    loggerPrintf(LOGGER_DEBUG, "Deleting element of ptr type 'const char *' at %lu\n", pos);
    // deletes string. see allocation in addElement function above...
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "String being deleted: '%s'\n", buf[pos]);
    if (buf != nullptr && buf[pos] != nullptr) {
        delete[] buf[pos];
    }
}

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

template<typename T>
void Array<T>::nlognsortMerge(T * A, size_t size_a, T * B, size_t size_b, T * swap_space) {
    size_t swap_space_push = 0;
    size_t swap_space_pop = 0;

    size_t i = 0;
    size_t j = 0;
    T swap;
    T left_compare;
    while (i < size_a) {
        left_compare = A[i];
        if (swap_space_push - swap_space_pop > 0) {
            left_compare = swap_space[swap_space_pop];
        }
        if (j < size_b && nlognsortCompare<T>(*this->e_sorted, left_compare, B[j]) > 0) {
            // B wins
            swap = A[i];
            A[i] = B[j];
            swap_space[swap_space_push++] = swap;
            j++;
        } else if (swap_space_push - swap_space_pop > 0) {
            // swap space wins
            swap = A[i];
            A[i] = swap_space[swap_space_pop++];
            // set new value at end of swap space
            swap_space[swap_space_push++] = swap;
        } // else swap_space empty and A wins
        i++;
    }
    // merge swap space with remaining B, remember assuming contigious
    while (swap_space_push - swap_space_pop > 0) {
        left_compare = swap_space[swap_space_pop];
        if (j < size_b && nlognsortCompare<T>(*this->e_sorted, left_compare, B[j]) > 0) {
            // by law of numbers i will never be more than j lol
            A[i] = B[j];
            j++;
        } else {
            // swap space wins
            A[i] = swap_space[swap_space_pop++];
            // note, size of swap space remains the same...
        } // else swap_space empty and A wins
        i++;
    }
    return;
}
template<typename T>
void Array<T>::nlognSort(T * e_buf, size_t size) {
    if (false == (e_buf == nullptr || size <= 1)) {
        // reduce memory usage size, recursion is generally frownd upon!
        size_t size_left = ceil(size/2.0);
        T * swap_space = new T[size_left];
        size_t span = 1;
        T * left_buf;
        T * right_buf;
        // so, this basically skips the first half of the tree... let's not bother updating other sort stuff because visualization..
        while (span < size) {
            size_t i = 0;
            while (i < size) {
                left_buf = e_buf + i;
                if (i + span < size) {
                    // if right buf is within bounds... 
                    //  else it's the odd element out (last element, so adhocly bring in the last odd element in later iterations.) 
                    right_buf = e_buf + i + span;
                    size_t right_size = span;
                    if (i + span + right_size > size) {
                        right_size = size - (i + span);
                    }
                    // left must always be larger or equal to right
                    merge(left_buf, span, right_buf, right_size, swap_space);
                }
                i += (2*span);
            }
            span *= 2;
        }
        delete[] swap_space;
    }
    return;
}

template<typename T>
Array<T>& Array<T>::sort(ArraySort sortOrder) {
    if (sortOrder != ARRAY_SORT_UNSORTED && *e_sorted != sortOrder) {
        *e_sorted = sortOrder;
        try {
            nlognSort(*this->e_buf, *this->e_size);
        } catch (const std::exception& e) {
            loggerPrintf(LOGGER_ERROR, "%s\n", e.what());
            // inserts, removes and bad allocs makes it so that we can't assume array is sorted.
            *e_sorted = ARRAY_SORT_UNSORTED;
            throw e;
        }
    }
    return *this;
}
template<typename T>
Array<T>& Array<T>::insert(const size_t pos, const T * els, const size_t num_els) {
    // pos out of bounds, return error...
    if (pos < 0 || pos > this->size()) {
        std::string msg = "Position out of range.";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

    loggerPrintf(LOGGER_DEBUG, "num_els: %ld, size: %ld, e_cap: %ld, pos: %ld\n", num_els, this->size(), this->cap(), pos);

    T * new_buf = *this->e_buf; 
    bool recapped = false;
    if (num_els + this->size() > this->cap()) {
        size_t new_cap = (size_t)((num_els + this->size()) * UPSIZE_FACTOR);
        new_buf = newCArray<T>(new_cap);
        if (new_buf == nullptr) {
            // if no bad_alloc thrown? lol whatever...
            std::string msg = "Failed to allocate new array.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        } else {
            recapped = true;
            *this->e_cap = new_cap;
            // if recapped, copy elements up until pos.
            //  the rest will be automagically initialized by the insert operation... (see use of new_buf vs this->buf variables below)
            size_t total_size_up_to_pos = pos * sizeof(T);
            for (size_t i = 0; i < pos; i++) {
                new_buf[i] = (*this->e_buf)[i];
            }
        }
    }

    T * bucket = newCArray<T>(num_els);
    if (bucket == nullptr) {
        // if no bad_alloc thrown? lol whatever...
        std::string msg = "Failed to allocate new array.";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    size_t bucket_push = 0;
    size_t bucket_pop = 0;
    for (size_t i = pos; i < this->size() + num_els; i++) {
        T value;
        if (i < pos + num_els) {
            value = els[i - pos];
        } else {
            value = bucket[bucket_pop];
            if (++bucket_pop == num_els) {
                bucket_pop = 0;
            }
        }
        if (i < this->size()) {
            bucket[bucket_push] = (*this->e_buf)[i];
            if (++bucket_push == num_els) {
                bucket_push = 0;
            }
        }
        addElement<T>(new_buf, i, value);
    }
    delete[] bucket;

    *this->e_size += num_els;

    if (recapped) {
        delete[] *this->e_buf;
        *this->e_buf = new_buf;
    }

    *e_sorted = ARRAY_SORT_UNSORTED;

    return *this;
}
template<typename T>
Array<T>& Array<T>::remove(const size_t pos, const size_t num_els) {
    // pos out of bounds, return error...
    if (pos < 0 || pos + num_els > this->size()) {
        std::string msg = "Position out of range.";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

    T * selected_buf = nullptr;
    bool recapped = false;
    size_t new_cap_threshold = this->cap() * DOWNSIZE_FACTOR;
    size_t potential_new_cap = ((this->size() - num_els) * UPSIZE_FACTOR);
    if (potential_new_cap < new_cap_threshold) {
        T * new_buf = newCArray<T>(potential_new_cap);
        // because of how new/delete work and we want to make sure the data is continuous, we'll need to reallocate...
        if (new_buf == nullptr) {
            // if no bad_alloc thrown? lol whatever...
            std::string msg = "Failed to allocate new array.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        } else {
            recapped = true;
            *this->e_cap = potential_new_cap;
            // if recapped, copy elements up until pos.
            //  the rest will be automatically intialized by remove operation... 
            selected_buf = new_buf;
            for (size_t i = 0; i < pos; i++) {
                selected_buf[i] = (*this->e_buf)[i];
            }
        }
    } else {
        // else, just remove, don't recap array...
        selected_buf = *this->e_buf;
    }
    for (size_t i = pos; i < this->size(); i++) {
        if (i < pos + num_els) {
            // make sure to deallocate memory for elements being removed.
            deleteCArrayElement<T>(*this->e_buf, i);
        }
        // if removing last element, no shifting needed... decrementing size should be enough...
        if (i + num_els < this->size()) {
            selected_buf[i] = (*this->e_buf)[i + num_els];
        }
    }
    if (recapped) {
        delete[] *this->e_buf;
        *this->e_buf = selected_buf;
    }

    *this->e_size -= num_els;

    return *this;
}