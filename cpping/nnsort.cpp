#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "logger.h"

static size_t nodes_visited = 0;

// TODO: sort order...
template<typename T>
void nnSort(T * e_buf, size_t size) {
    if (e_buf == nullptr || size <= 1) {
        return;
    } else {
        T swap;
        for (size_t i = 0; i < size; i++) {
            for (size_t j = 0; j < size - i; j++) {
                if (e_buf[j] > e_buf[j+1]) {
                    swap = e_buf[j];
                    e_buf[j] = e_buf[j+1];
                    e_buf[j+1] = swap;
                }
                nodes_visited++;
            }
        }
    }
}

void generateRandomArray(int * array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        // array[i] = rand();
        // normalized to size of array for easier visualization
        array[i] = (int)round(size * (double)rand()/RAND_MAX);
    }
}

void printArray(int * array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("[%d]", array[i]);
    }
    printf("\n");
}

#define ARRAY_SIZE 77
// #define ARRAY_SIZE 7

int main() {
    int array[ARRAY_SIZE];
    generateRandomArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    struct timespec ts_before;
    clock_gettime(CLOCK_MONOTONIC, &ts_before);

    nodes_visited = 0;
    nnSort<int>(array, ARRAY_SIZE);

    loggerPrintf(LOGGER_TEST, "NODES VISITED: %ld\n", nodes_visited);
    printArray(array, ARRAY_SIZE);

    struct timespec ts_after;
    clock_gettime(CLOCK_MONOTONIC, &ts_after);

    loggerPrintf(LOGGER_TEST, "RUNTIME_s: %lu, RUNTIME_ns: %lu\n", ts_after.tv_sec - ts_before.tv_sec, ts_after.tv_nsec - ts_before.tv_nsec);

    return 1;
}