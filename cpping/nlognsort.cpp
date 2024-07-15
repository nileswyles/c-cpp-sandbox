#include "nlognsort.h"

template<typename T>
// let's assume these are contiguous
void merge(T * A, size_t sizeA, T * B, size_t sizeB, T * swap_space) {
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

    // if A always wins, 
    //  then j = 0;
    //  swap_space == 0;
    //  i = sizeA
    //  and so, B is just appended to A... since they are contigious nothing needs to happen...

    // 
    // okay so, if B wins always
    //  then j == sizeB
    //  swap_space == sizeB
    //  i = sizeA
    //  and so, swap_space is already sorted and you should be able to assume you can just append to A
    //  effectively swapping B with A...

    // hybrid, simlilar as above?

    //  B wins twice
    //  j == 2
    //  swap_space == 2
    //  i = sizeA
    // and so, we want to compare swap_space with B and place in A... 
    //  by laws of laws... swap_space wins 
    //  assuming swap space wins twice...
    //  nothing needs to happen because contigious.

    //  assuming swapspace wins 1 B wins 1 then swap space remaining is then also just B... so, yeah that check works.

    //  S, S
    //  0, 0, B, B

    //  S
    //  S, 0, B, B

    // S
    // S, B, B, B

    // []
    // S, B, S, B

    //  by laws of laws... this should extend to other array sizes right?

    //  S, S
    //  0, 0, B, B, B

    //  S
    //  S, 0, B, B, B

    // S
    // S, B, B, B, B

    // []
    // S, B, S, B, B

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

// TODO: sort order...
// nlognsort
template<typename T>
Agnode_t * nlognSort(Agraph_t * g, Agnode_t * parent_node, T * e_buf, size_t size, T * ss) {
    if (e_buf == nullptr || size < 1) {
        return nullptr;
    } else if (size == 1) {
        return drawNode<T>(g, parent_node, e_buf, size);
    } else {
        // ensure left always larger than right
        size_t size_left = ceil(size/2.0);
        size_t size_right = size - size_left;
        loggerPrintf(LOGGER_DEBUG, "CALL TRACE: size: %ld, left: %ld, right: %ld\n", size, size_left, size_right);
        T * left_buf = e_buf;
        T * right_buf = e_buf + size_left;
        // 0 + 2 = 2

        // 0, 1
        // 2, 3

        // 0 + 3 = 3
        // 0, 1, 2

        // 3, 4
        Agnode_t * left;
        Agnode_t * right;
        drawUnsorted<T>(g, parent_node, &left, &right, left_buf, size_left, right_buf, size_right);

        T * swap_space = ss;
        if (ss == nullptr) {
            swap_space = new T[size_left];
        }
        Agnode_t * left_sorted = nlognSort<T>(g, left, left_buf, size_left, swap_space); // left
        Agnode_t * right_sorted = nlognSort<T>(g, right, right_buf, size_right, swap_space); // right
        merge<T>(left_buf, size_left, right_buf, size_right, swap_space);
        if (ss == nullptr) {
            delete[] swap_space;
        }
        loggerPrintf(LOGGER_DEBUG, "CALL TRACE merged size: %ld\n", size);
        return drawMergedNode<T>(g, left_sorted, right_sorted, e_buf, size);
    }
}

template<typename T>
void nlognSort(T * e_buf, size_t size, T * ss) {
    if (e_buf == nullptr || size <= 1) {
        return;
    } else {
        // ensure left always larger than right
        size_t size_left = ceil(size/2.0);
        size_t size_right = size - size_left;
        T * left_buf = e_buf;
        T * right_buf = e_buf + size_left;
        loggerPrintf(LOGGER_DEBUG, "CALL TRACE: size: %ld, left: %ld, right: %ld\n", size, size_left, size_right);
        T * swap_space = ss;
        if (ss == nullptr) {
            swap_space = new T[size_left];
        }
        nlognSort<T>(left_buf, size_left, swap_space); // left
        nlognSort<T>(right_buf, size_right, swap_space); // right
        merge<T>(left_buf, size_left, right_buf, size_right, swap_space);
        if (ss == nullptr) {
            // delete[] swap_space;
        }
        loggerPrintf(LOGGER_DEBUG, "CALL TRACE merged size: %ld\n", size);
    }
}

int main(int argc, char **argv) {
    int array[ARRAY_SIZE];
    generateRandomArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    int unsorted[ARRAY_SIZE];
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        unsorted[i] = array[i];
    }

    struct timespec ts_before;
    clock_gettime(CLOCK_MONOTONIC, &ts_before);
#if GRAPH_ENABLE
    GVC_t * gvc = graphInit(argc, argv);
    Agraph_t * g; 
    /* Create a simple digraph */
    g = agopen("G", Agdirected, nullptr);
    Agnode_t * root = drawNode<int>(g, nullptr, array, ARRAY_SIZE);
    nodes_visited = 0;
    nlognSort<int>(g, root, array, ARRAY_SIZE, nullptr);
#else
    nodes_visited = 0;
    nlognSort<int>(array, ARRAY_SIZE, nullptr);
#endif
    loggerPrintf(LOGGER_TEST, "NODES VISITED: %ld\n", nodes_visited);
    printArray(array, ARRAY_SIZE);

    struct timespec ts_after;
    clock_gettime(CLOCK_MONOTONIC, &ts_after);

    loggerPrintf(LOGGER_TEST, "RUNTIME_s: %lu, RUNTIME_ns: %lu\n", ts_after.tv_sec - ts_before.tv_sec, ts_after.tv_nsec - ts_before.tv_nsec);

    loggerPrintf(LOGGER_TEST, "ARRAY MATCH: %s\n", compareArrays<int>(unsorted, ARRAY_SIZE, array, ARRAY_SIZE) == 0 ? "FALSE" : "TRUE");
#if GRAPH_ENABLE
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