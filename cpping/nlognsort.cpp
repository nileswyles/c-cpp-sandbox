#include "nlognsort.h"

#define GRAPH_ENABLE 1

template<typename T>
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
        printf("i: %ld, swap_space_size(j): %ld, A[i]: %ld, B[j]: %ld, SS[i]: %ld\n", i, swap_space_size, A[i], B[swap_space_size], swap_space[0]);
        if (j < sizeB && left_compare > B[j]) {
            // B wins
            swap = A[i];
            A[i] = B[j];
            swap_space[swap_space_size] = swap;

            swap_space_size++;
            j++;
            // note, swap space extends further into B space...
        } else if (swap_space_size > 0) {
            swap = A[i];
            A[i] = swap_space[0];
            // unnavoidable shifting...
            size_t index_of_last_element = swap_space_size - 1;
            for (size_t i = 0; i < index_of_last_element; i++) {
                swap_space[i] = swap_space[i + 1];
                nodes_visited++;
            }
            // set new value at end of swap space
            swap_space[index_of_last_element] = swap;

            // note, size of swap space remains the same...
        } // else swap_space empty and A wins

        // okay, now if swap_space is empty....
        //  will it every empty past original A but before extensions consumed?
        printf("WINNER WINNER: %ld\n", A[i]);
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
    bool first_run = true;
    // then again. if swap_space isn't empty merge swap_space with remaining B
    while (first_run || swap_space_size > 1) {
        first_run = false;

        A = swap_space;
        sizeA = swap_space_size;
        B = &B[swap_space_size];
        swap_space = B;
        sizeB -= swap_space_size;
        swap_space_size = 0;
        leftMerge(A, sizeA, B, sizeB, swap_space, swap_space_size);
    }
}

// #define ARRAY_SIZE 7
#define ARRAY_SIZE 10
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
    root = drawMultiValueNode(g, nullptr, array, ARRAY_SIZE);
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