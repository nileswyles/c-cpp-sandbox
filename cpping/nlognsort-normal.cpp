#include "nlognsort.h"

#define GRAPH_ENABLE 1

template<typename T>
void merge(T * A, size_t sizeA, T * B, size_t sizeB, T ** merged_container) {
    size_t i = 0;
    size_t j = 0;
    size_t o = 0;

    T * out = new T[sizeA+sizeB];
    printf("lol\n");
    while (o < sizeA+sizeB) {
        if (A[i] > B[j]) {
            out[o++] = B[j++];
        } else {
            out[o++] = A[i++];
        }
    }
    printf("lol\n");
    // delete A;
    // delete B;
    printf("lol\n");
    *merged_container = out;
}

#define ARRAY_SIZE 4

int main() {
    int array[ARRAY_SIZE];
    generateRandomArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);
    int * sorted;
    nlognSort<int>(array, ARRAY_SIZE, &sorted);
    printArray(sorted, ARRAY_SIZE);

    return 1;
}