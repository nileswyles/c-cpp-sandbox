#include "nlognsort.h"

// TODO: compare with other sorting methods...
//  I expect this to outpeform the others, on average (ironically enough - trust the simulation?)...
//  nlognsort-normal might outperform this when there are a lot of swap space wins

// another, sort of hybrid approach (which was sort of the direction I was headed in), is to use a statically allocated swap array?
//  maybe use swap data from this impl to determine size. Downside, locking, static storage for each type? 

// or, instead dynamically allocate swap space? the other hybrid solution?
//  pros:
//      - less new/delete churn compared to nlognsort-normal 
//      - less space needed compared to nlognsort-normal
//      - no need to update pointer in container. 
//
//  by all measures better than nlognsort-normal?
// lol... I think so... hmm....

static size_t SWAP_SPACE_SHIFT_DATA[ARRAY_SIZE];
static size_t SWAP_SPACE_SHIFT_DATA_IDX = 0;

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
            loggerPrintf(LOGGER_DEBUG, "Shifting %lu elements\n", index_of_last_element);
            for (size_t i = 0; i < index_of_last_element; i++) {
                swap_space[i] = swap_space[i + 1];
                nodes_visited++;
            }
            // set new value at end of swap space
            swap_space[index_of_last_element] = swap;

            // Compiler should otherwise optimize this out...
            if (DATA_COLLECTION) {
                SWAP_SPACE_SHIFT_DATA[SWAP_SPACE_SHIFT_DATA_IDX++] = swap_space_size;
 
                // collect data on shift amounts...
                //  plot (average?) against length of array?
                //  linear regression?
 
                // lol... actually, ironically enough, outliers/extremes/worst-case and best-case are what matter
                // hmm... sstill would be intersting to get the hybrid solution working, how well thought out was the generics feature?
                //      can just set a hard max on original array size...
                //      and/or assume swap_space isn't going to be more than half of original array size.... and seet hard limit there
                //      or or or, account for 1 std deviation? and throw runtime exception?
 
                //  likely the former because it's more predictable (ding ding ding, winner winner), but, still curious what that distribution looks like? heavily reliant on random function? Idk stats...

                // decisions decisions
            }


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

// TODO: sort order...
// nlognsort
template<typename T>
Agnode_t * nlognSort(Agraph_t * g, Agnode_t * parent_node, T * e_buf, size_t size) {
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

        Agnode_t * left;
        Agnode_t * right;
        drawUnsorted<T>(g, parent_node, &left, &right, left_buf, size_left, right_buf, size_right);

        Agnode_t * left_sorted = nlognSort<T>(g, left, left_buf, size_left); // left
        Agnode_t * right_sorted = nlognSort<T>(g, right, right_buf, size_right); // right
        merge<T>(left_buf, size_left, right_buf, size_right);
        loggerPrintf(LOGGER_DEBUG, "CALL TRACE merged size: %ld\n", size);
        return drawMergedNode<T>(g, left_sorted, right_sorted, e_buf, size);
    }
}

template<typename T>
void nlognSort(T * e_buf, size_t size) {
    if (e_buf == nullptr || size <= 1) {
        return;
    } else {
        // ensure left always larger than right
        size_t size_left = ceil(size/2.0);
        size_t size_right = size - size_left;
        T * left_buf = e_buf;
        T * right_buf = e_buf + size_left;
        loggerPrintf(LOGGER_DEBUG, "CALL TRACE: size: %ld, left: %ld, right: %ld\n", size, size_left, size_right);
        nlognSort<T>(left_buf, size_left); // left
        nlognSort<T>(right_buf, size_right); // right
        merge<T>(left_buf, size_left, right_buf, size_right);
        loggerPrintf(LOGGER_DEBUG, "CALL TRACE merged size: %ld\n", size);
    }
}

int main(int argc, char **argv) {
    int array[ARRAY_SIZE];
    generateRandomArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    struct timespec ts_before;
    clock_gettime(CLOCK_MONOTONIC, &ts_before);
#ifdef GRAPH_ENABLE
    GVC_t * gvc = graphInit(argc, argv);
    Agraph_t * g; 
    /* Create a simple digraph */
    g = agopen("G", Agdirected, nullptr);
    Agnode_t * root = drawNode<int>(g, nullptr, array, ARRAY_SIZE);
    nodes_visited = 0;
    nlognSort<int>(g, root, array, ARRAY_SIZE);
#else
    nodes_visited = 0;
    nlognSort<int>(array, ARRAY_SIZE);

#endif
    loggerPrintf(LOGGER_TEST, "NODES VISITED: %ld\n", nodes_visited);
    printArray(array, ARRAY_SIZE);

    struct timespec ts_after;
    clock_gettime(CLOCK_MONOTONIC, &ts_after);

    loggerPrintf(LOGGER_TEST, "RUNTIME_s: %lu, RUNTIME_ns: %lu\n", ts_after.tv_sec - ts_before.tv_sec, ts_after.tv_nsec - ts_before.tv_nsec);

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