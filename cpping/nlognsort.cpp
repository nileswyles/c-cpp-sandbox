#include "nlognsort.h"

#define GRAPH_ENABLE 1

// TODO: compare with other sorting methods...
//  I expect this to outpeform the others, on average (ironically enough - trust the simulation?)...
//  nlognsort-normal might outperform this when there a lot of swap space wins

// another, sort of hybrid approach (which was sort of the direction I was headed in), is to use a statically allocated swap array?
//  maybe use swap data from this impl to determine size. Downside, locking, static storage for each type? 

template<typename T>
// TODO: lol reference of pointer? seems reasonable?
void leftMerge(T * A, size_t sizeA, T * B, size_t sizeB, T *& swap_space, size_t& swap_space_size) {
    size_t i = 0;
    size_t j = 0;
    T swap;
    T left_compare;
    while (i < sizeA) {
        left_compare = A[i];
        if (swap_space_size > 0) {
            left_compare = swap_space[0];
        }
        // TODO: 65535 when sizeB == 0
        // printf("i: %ld, j: %ld, A[i]: %ld, B[j]: %ld, SS[i]: %ld, sizeB: %ld\n", i, j, A[i], B[j], swap_space[0], sizeB);
        if (j < sizeB && left_compare > B[j]) {
            // B wins
            swap = A[i];
            A[i] = B[j];
            swap_space[swap_space_size] = swap;

            swap_space_size++;
            j++;
            // note, swap space extends further into B space...
        } else if (swap_space_size > 0) {
            // swap space wins
            swap = A[i];
            A[i] = swap_space[0];
            // unavoidable shifting...
            size_t index_of_last_element = swap_space_size - 1;
            for (size_t i = 0; i < index_of_last_element; i++) {
                swap_space[i] = swap_space[i + 1];
                nodes_visited++;
            }
            // set new value at end of swap space
            swap_space[index_of_last_element] = swap;

            // note, size of swap space remains the same...
        } // else swap_space empty and A wins
        i++;
        nodes_visited++;
    }
}

template<typename T>
// let's assume these are contiguous
void merge(T * A, size_t sizeA, T * B, size_t sizeB) {
    size_t swap_space_size = 0;
    T * swap_space = B;
    leftMerge(A, sizeA, B, sizeB, swap_space, swap_space_size);
    // then again. if swap_space isn't empty merge swap_space with remaining B
    // and might want to round up in split function in caller function, so that A is always larger than B... otherwise, last value will never be compared
    while (swap_space_size > 0) {
        A = swap_space;
        sizeA = swap_space_size;
        B = &B[swap_space_size];
        swap_space = B;
        sizeB -= swap_space_size;
        swap_space_size = 0;
        leftMerge(A, sizeA, B, sizeB, swap_space, swap_space_size);
    }
}

#define ARRAY_SIZE 27
// #define ARRAY_SIZE 77
//TODO: 
// random not working?

// #define ARRAY_SIZE 6

int main(int argc, char **argv) {
    int array[ARRAY_SIZE];
    generateRandomArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);
#ifdef GRAPH_ENABLE
    GVC_t * gvc = graphInit(argc, argv);
    Agraph_t * g; 
    /* Create a simple digraph */
    g = agopen("G", Agdirected, nullptr);
    Agnode_t * root;
    root = drawNode<int>(g, nullptr, array, ARRAY_SIZE);
    nodes_visited = 0;
    nlognSort<int>(g, root, array, ARRAY_SIZE);
#else
    nodes_visited = 0;
    nlognSort<int>(array, ARRAY_SIZE);

#endif
    printf("NODES VISITED: %ld\n", nodes_visited);
    printArray(array, ARRAY_SIZE);

#ifdef GRAPH_ENABLE
    /* Compute a layout using layout engine from command line args */
    gvLayoutJobs(gvc, g);
    /* Write the graph according to -T and -o options */
    gvRenderJobs(gvc, g);

    /* Free layout data */
    gvFreeLayout(gvc, g);
    /* Free graph structures */
    agclose(g);

    /* close output file, free context, and return number of errors */
    return (gvFreeContext(gvc));
#else
    return 0;
#endif
}