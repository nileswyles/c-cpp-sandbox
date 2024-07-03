#include "nlognsort.h"

#define GRAPH_ENABLE 1

void leftMerge() {
    T swap;
    T left_compare;
    while (i < sizeA) {
        left_compare = A[i];
        if (swap_space_size > 0) {
            left_compare = swap_space[0];
        }
        printf("i: %ld, j: %ld, A[i]: %ld, B[j]: %ld, SS[i]: %ld\n", i, j, A[i], B[j], swap_space[0]);
        if (j < sizeB && left_compare > B[j]) {
            // B wins
            swap = A[i];
            A[i] = B[j];

            // if (&(A[i]) == swap_space) {
            //     // edge case where, start of swap_space == A[i] (output slot)...
            //     //  B[j] is unoccupied and equals swap_space[swap_space_slot] after incrementing...
            //     //  so, A[i]/originally swapped element get's added to the end...

            //     //   breaking sorted assumption...
            //     //  hmm....
            //     //  
            //     // but that's not the issue I am seeing here lol...
            //     //  let's get back to this. it's an issue  introduced by iterating past end of sizeA...
            //     // so, let's sort sizeB seperatly using nn sort?
            //     swap_space++;
            //     swap_space[swap_space_slot] = swap;
            // } else {
                swap_space[swap_space_slot++] = swap;
                swap_space_size++;
            // }
            j++;
        } else if (swap_space_size > 0) {
            // swap_space wins
            swap = A[i];
            A[i] = swap_space[0];
            swap_space[swap_space_slot] = swap;
            swap_space++;
            // size of swap space remains the same.
        } // else swap_space empty and A wins
        printf("WINNER WINNER: %ld\n", A[i]);
        i++;
        nodes_visited++;
    }
}

template<typename T>
// let's assume these are contiguous
void merge(T * A, size_t sizeA, T * B, size_t sizeB) {
    size_t i = 0;
    size_t j = 0;

    T * swap_space = B;
    size_t swap_space_size = 0;
    size_t swap_space_slot = 0;
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
        if (swap_space_size > 0) {
            left_compare = swap_space[0];
        }
        printf("i: %ld, j: %ld, A[i]: %ld, B[j]: %ld, SS[i]: %ld\n", i, j, A[i], B[j], swap_space[0]);
        if (j < sizeB && left_compare > B[j]) {
            // B wins
            swap = A[i];
            A[i] = B[j];

            // if (&(A[i]) == swap_space) {
            //     // edge case where, start of swap_space == A[i] (output slot)...
            //     //  B[j] is unoccupied and equals swap_space[swap_space_slot] after incrementing...
            //     //  so, A[i]/originally swapped element get's added to the end...

            //     //   breaking sorted assumption...
            //     //  hmm....
            //     //  
            //     // but that's not the issue I am seeing here lol...
            //     //  let's get back to this. it's an issue  introduced by iterating past end of sizeA...
            //     // so, let's sort sizeB seperatly using nn sort?
            //     swap_space++;
            //     swap_space[swap_space_slot] = swap;
            // } else {
                swap_space[swap_space_slot++] = swap;
                swap_space_size++;
            // }
            j++;
        } else if (swap_space_size > 0) {
            // swap_space wins
            swap = A[i];
            A[i] = swap_space[0];
            swap_space[swap_space_slot] = swap;
            swap_space++;
            // size of swap space remains the same.
        } // else swap_space empty and A wins
        printf("WINNER WINNER: %ld\n", A[i]);
        i++;
        nodes_visited++;
    }

    // hmmm... if can assume swap_space is sorted,, then it's basically same as above... 
    // if not, the nn sort?

    // AAAA BBBB
    // AABA ABBB
    // AABB AABB

    // AA BB
    // BA1 AB1

    //  A1 > B1
    // BA 
    // again, assuming contiguous
    //      also, A[i] == swap_space[0]...
    i = 0;
    A = swap_space;
    sizeA = swap_space_size;
    swap_space = &B[j];
    swap_space_size = 0;
    swap_space_slot = 0;
    while (i < sizeA) {
        left_compare = A[i];
        if (swap_space_size > 0) {
            left_compare = swap_space[0];
        }
        printf("i: %ld, j: %ld, A[i]: %ld, B[j]: %ld, SS[i]: %ld\n", i, j, A[i], B[j], swap_space[0]);
        if (j < sizeB && left_compare > B[j]) {
            // B wins
            swap = A[i];
            A[i] = B[j];

            swap_space[swap_space_slot++] = swap;
            swap_space_size++;
            j++;
        } else if (swap_space_size > 0) {
            // swap_space wins
            swap = A[i];
            A[i] = swap_space[0];
            swap_space[swap_space_slot] = swap;
            swap_space++;
            // size of swap space remains the same.
        } // else swap_space empty and A wins
        printf("WINNER WINNER: %ld\n", A[i]);
        i++;
        nodes_visited++;
    }

    // while (j < sizeB && swap_space_size > 0) {
    //     printf("in B/swap space --- i: %ld, j: %ld, A[i]: %ld, B[j]: %ld, SS[i]: %ld\n", i, j, A[i], B[j], swap_space[0]);
    //     if (swap_space[0] > B[j]) {
    //         // B wins
    //         swap = swap_space[0];
    //         swap_space[0] = B[j];
    //     } else {

    //     }
    //     printf("WINNER WINNER: %ld\n", swap_space[0]);
    //     swap_space[swap_space_slot] = swap;
    //     printf("swap_space_slot: %ld\n", swap_space_slot);
    //     printf("swap_space[swap_space_slot]: %ld, B[j]: %ld\n", swap_space[0], B[j]);
    //     swap_space++;
    //     // size of swap space remains the same.
    //     nodes_visited++;
    // }

    // if odd size before split,
    // j may not equal sizeB but that's fine because it should already be sorted... 

    // Yeah, so now the question is whether these extra operations are worse than allocating a new array...
    //  probably splitting hairs (hares?) lol - but interesting...
}

// #define ARRAY_SIZE 7
#define ARRAY_SIZE 4
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