#include <stdlib.h>
#include <stdio.h>

template<typename T>
T * merge(T * A, size_t sizeA, T * B, size_t sizeB) {
    size_t i = 0;
    size_t j = 0;
    size_t o = 0;

    T * out = new T[sizeA+sizeB];
    while (o < sizeA+sizeB) {
        if (A[i] > B[j]) {
            out[o++] = B[j++];
        } else {
            out[o++] = A[i++];
        }
    }
    delete A;
    delete B;
    return out;
}

// TODO: sort order...
template<typename T>
T * nlognSort(T * e_buf, size_t size) {
    if (e_buf == nullptr || size <= 1) {
        return;
    } else {
        size_t split_index = size/2;
        T * A = nlognSort<T>(e_buf, split_index); // left
        T * B = nlognSort<T>(e_buf + split_index, size - split_index); // right
        return merge<T>(A, split_index, B, size - split_index);
    }
}

void generateRandomArray(int * array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        array[i] = rand();
    }
}

void printArray(int * array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("[%d]", array[i]);
    }
    printf("\n");
}

#define ARRAY_SIZE 7

int main() {
    int array[ARRAY_SIZE];
    generateRandomArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);
    int * sorted = nlognSort<int>(array, ARRAY_SIZE);
    printArray(sorted, ARRAY_SIZE);

    return 1;
}