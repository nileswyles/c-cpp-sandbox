#include <stdlib.h>
#include <stdio.h>

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
            }
        }
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
    nnSort<int>(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    return 1;
}