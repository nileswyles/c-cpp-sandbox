#include <stdlib.h>
#include <stdio.h>

template<typename T>
// let's assume these are contiguous
void merge(T * A, size_t sizeA, T * B, size_t sizeB) {
    size_t i = 0;
    size_t j = 0;

    T * swap_space = B;
    size_t swap_space_size = 0;
    size_t swap_space_i = 0;
    // so, goal is to not allocate

    // output == A[i];
    // compare A[i] and B[j]
    //  A[i] == larger/smaller of two...
    //  if A[i] larger increment i,
    //  if B[j], A[i] goes into swap space, increment i and j

    // then going forward...
    // or rather, also, before comparing with A[i], we check swap space...
    // so, let's say, left compare = A[i] if swap_space_size == 0;
    // else left compare = swap_space[swap_space_i];

    // TODO: think about if odd size... in other words, sizeB < sizeA

    // because who knows compiler sstuff... :)
    // this is the more portable way or better way of making sure a new variable isn't placed on the stack for each iteration?
    // is this a basic requirement of modern compilers?

    // back in the day needed to define all variables at start of function.
    T swap;
    T left_compare;
    while (i < sizeA) {
        left_compare = A[i];
        if (swap_space_size != 0) {
            left_compare = swap_space[0];
        }
        if (j < sizeB && left_compare > B[j]) {
            // B wins
            swap = A[i];
            A[i] = B[j];
            swap_space[swap_space_i++] = swap;
            swap_space_size++;
            j++;
        } else if (swap_space_size > 0) {
            // swap_space wins
            swap = A[i];
            A[i] = swap_space[0];
            swap_space++;
            swap_space[swap_space_i++] = swap;
            // size of swap space remains the same.
        } // else swap_space empty and A wins
        i++;
    }

    // if odd size before split,
    // j may not equal sizeB but that's fine because it should already be sorted... 

    // Yeah, so now the question is whether these extra operations are worse than allocating a new array...
    //  probably splitting hairs (hares?) lol - but interesting...
}

// TODO: sort order...
template<typename T>
void nlognSort(T * e_buf, size_t size) {
    if (e_buf == nullptr || size <= 1) {
        return;
    } else {
        size_t split_index = size/2;
        nlognSort<T>(e_buf, split_index); // left
        nlognSort<T>(e_buf + split_index, size - split_index); // right
        merge<T>(e_buf, split_index, e_buf + split_index, size - split_index);
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
    nlognSort<int>(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    return 1;
}