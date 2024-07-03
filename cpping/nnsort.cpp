#include <stdlib.h>
#include <stdio.h>

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
        array[i] = rand();
    }
}

void printArray(int * array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("[%d]", array[i]);
    }
    printf("\n");
}

#define ARRAY_SIZE 17
// #define ARRAY_SIZE 7

int main() {
    int array[ARRAY_SIZE];
    generateRandomArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);
    nodes_visited = 0;
    nnSort<int>(array, ARRAY_SIZE);
    printf("NODES VISITED: %ld\n", nodes_visited);
    printArray(array, ARRAY_SIZE);

    return 1;
}