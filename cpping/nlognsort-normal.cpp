#include "nlognsort.h"

template<typename T>
T * merge(T * A, size_t sizeA, T * B, size_t sizeB) {
    size_t i = 0;
    size_t j = 0;
    size_t o = 0;

    T * out = new T[sizeA+sizeB];
    while (o < sizeA+sizeB) {
        loggerPrintf(LOGGER_DEBUG, "%i, %i\n", A[i], B[j]);
        if (j >= sizeB) {
            out[o++] = A[i++];
        } else if (i >= sizeA) {
            out[o++] = B[j++];
        } else if (A[i] > B[j]) {
            out[o++] = B[j++];
        } else {
            out[o++] = A[i++];
        }
        loggerPrintf(LOGGER_DEBUG, "winner: %i\n", out[o - 1]);
        nodes_visited++;
    }
    // delete A;
    // delete B;
    return out;
}

// nlognsort-normal
// lol, yeah maybe move these to cpp file and forget this reference stuff...
template<typename T>
T * nlognSort(Agraph_t * g, Agnode_t * parent_node, T * e_buf, size_t size, Agnode_t ** resulting_node) {
    if (e_buf == nullptr || size < 1) {
        return nullptr;
    } else if (size == 1) {
        *resulting_node = drawNode<T>(g, parent_node, e_buf, size);
        return e_buf;
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

        Agnode_t * left_sorted;
        Agnode_t * right_sorted;
        T * A = nlognSort<T>(g, left, left_buf, size_left, &left_sorted); // left
        T * B = nlognSort<T>(g, right, right_buf, size_right, &right_sorted); // right

        T * result = merge<T>(A, size_left, B, size_right);
        *resulting_node = drawMergedNode<T>(g, left_sorted, right_sorted, result, size);
        loggerPrintf(LOGGER_DEBUG, "CALL TRACE merged size: %ld\n", size);
        return result;
    }
}

template<typename T>
T * nlognSort(T * e_buf, size_t size) {
    if (e_buf == nullptr || size <= 1) {
        return e_buf;
    } else {
        // ensure left always larger than right
        size_t size_left = ceil(size/2.0);
        size_t size_right = size - size_left;
        T * left_buf = e_buf;
        T * right_buf = e_buf + size_left;

        T * A = nlognSort<T>(left_buf, size_left); // left
        T * B = nlognSort<T>(right_buf, size_right); // right
        return merge<T>(A, size_left, B, size_right);
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
    Agnode_t * resulting_node;

    nodes_visited = 0;
    int * sorted = nlognSort<int>(g, root, array, ARRAY_SIZE, &resulting_node);
#else
    nodes_visited = 0;
    int * sorted = nlognSort<int>(array, ARRAY_SIZE);
#endif
    loggerPrintf(LOGGER_TEST, "NODES VISITED: %ld\n", nodes_visited);
    printArray(sorted, ARRAY_SIZE);

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
    return 1;
}