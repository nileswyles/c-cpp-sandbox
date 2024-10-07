#include "nlognsort.h"

template<typename T>
// let's assume these are contiguous
inline void merge(T * A, size_t sizeA, T * B, size_t sizeB, T * swap_space) {
    size_t swap_space_push = 0;
    size_t swap_space_pop = 0;

    size_t i = 0;
    size_t j = 0;
    T swap;
    T left_compare;
    while (i < sizeA) {
        left_compare = A[i];
        if (swap_space_push - swap_space_pop > 0) {
            left_compare = swap_space[swap_space_pop];
        }
        if (j < sizeB && left_compare > B[j]) {
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
        nodes_visited++;
    }

    // merge swap space with remaining B, remember assuming contigious
    while (swap_space_push - swap_space_pop > 0) {
        left_compare = swap_space[swap_space_pop];
        if (j < sizeB && left_compare > B[j]) {
            // by law of numbers i will never be more than j lol
            A[i] = B[j];
            j++;
        } else {
            // swap space wins
            A[i] = swap_space[swap_space_pop++];
            // note, size of swap space remains the same...
        } // else swap_space empty and A wins
        i++;
        nodes_visited++;
    }
}

template<typename T>
void nlognSort(T * e_buf, size_t size, T * ss) {
    if (e_buf == nullptr || size <= 1) {
        return;
    } else {
        // reduce memory usage size, recursion is generally frownd upon!
        size_t size_left = ceil(size/2.0);
        T * swap_space = new T[size_left];
        size_t span = 1;
        T * left_buf;
        T * right_buf;
        // so, this basically skips the first half of the tree... let's not bother updating other sort stuff with visualization..
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
        // so, this implementation should go like this...
        //      7, 4, 1, 9, 5
            
        //      7<>4     1<>9     5
        // =>   4,7      1,9      5
        //      4,7<>1,9     5
        // =>   1,4,7,9      5

        //      1,4,7,9<>5
        // =>   1,4,5,7,9

        delete[] swap_space;
    }
}

int main(int argc, char **argv) {
    int * array = new int[ARRAY_SIZE];
    generateRandomArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    int * unsorted = new int[ARRAY_SIZE];
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        unsorted[i] = array[i];
    }

    struct timespec ts_before;
    clock_gettime(CLOCK_MONOTONIC, &ts_before);
    nodes_visited = 0;
    nlognSort<int>(array, ARRAY_SIZE, nullptr);
    loggerPrintf(LOGGER_TEST, "NODES VISITED: %ld\n", nodes_visited);
    printArray(array, ARRAY_SIZE);

    struct timespec ts_after;
    clock_gettime(CLOCK_MONOTONIC, &ts_after);

    loggerPrintf(LOGGER_TEST, "RUNTIME_s: %lu, RUNTIME_ns: %lu\n", ts_after.tv_sec - ts_before.tv_sec, ts_after.tv_nsec - ts_before.tv_nsec);

    loggerPrintf(LOGGER_TEST, "ARRAY MATCH: %s\n", compareArrays<int>(unsorted, ARRAY_SIZE, array, ARRAY_SIZE) == 0 ? "FALSE" : "TRUE");

    delete[] array;
    delete[] unsorted;

    return 0;
}