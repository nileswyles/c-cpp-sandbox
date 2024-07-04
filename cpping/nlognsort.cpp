#include "nlognsort.h"

#define GRAPH_ENABLE 1

template<typename T>
size_t leftMerge(T * A, size_t sizeA, T * B, size_t sizeB, T *& swap_space, size_t& swap_space_size) {
    size_t i = 0;
    size_t j = 0;
    T swap;
    T left_compare;
    
    size_t extension = 0;
    while (i < sizeA + extension) {
        left_compare = A[i];
        if (swap_space_size > 0) {
            left_compare = swap_space[0];
        }
        printf("i: %ld, swap_space_size(j): %ld, A[i]: %ld, B[j]: %ld, SS[i]: %ld\n", i, swap_space_size, A[i], B[swap_space_size], swap_space[0]);
        if (j < sizeB && left_compare > B[j]) {
            // B wins
            if (i < sizeA) {
                swap = A[i];
                A[i] = B[j];
                swap_space[swap_space_size] = swap;

                swap_space_size++;
                j++;
            } else {
                // writing past original A array and into original swap_space/start of B (contiguous)

                // here we just set the "deallocated" value to value at B and increment.
                // this continues until B wins consecutively - extension number of times
                // this also implies that when iterating past original A array, swap_space is never empty and the left_compare logic above is still valid... 
                A[i] = B[j];
                j++;
            }
        } else if (swap_space_size > 0) {
            if (i < sizeA) {
                // swap_space wins
                swap = A[i];
                A[i] = swap_space[0];
                // hmm...
                // so, we don't want the gap...
                //  how do I elimate that? sizeA++ isn't actually fixing that...
 
                // so, if swap_space_wins, we place value at A[i] location at end of swap_space (preserving orderliness LMAO), that's a strict requirement...
                // so, I guess only really solution is to track gaps...?
 
                // actually, original idea was to continue iterating so that 
                //  as alternative to explicitly tracking the gaps and passing that info across invocations of this function...
                //  what if, I check if past original sizeA, and if so, don't swap... just consume swap_space...
                //  algorithm works as normally expected and 
                //  then no gap, :)
                swap_space++;
                swap_space[swap_space_size] = swap;
                // size of swap space remains the same.
            } else {
                // writing past original A array and into original swap_space/start of B (contiguous)

                // swap space won at some point and need to preserve orderliness, so in order to avoid gap, let's extend iterations and just consume swap space if beyond original sizeA (again, assuming contiguous)
                // here we just consume swap_space and similiar to the normal case tell the algorithm go even further and write to an extra value past original A (extension++).
                A[i] = swap_space[0];
                swap_space++;
                swap_space_size--;
            }
            extension++;
        } // else swap_space empty and A wins

        // okay, now if swap_space is empty....
        //  will it every empty past original A but before extensions consumed?
        printf("WINNER WINNER: %ld\n", A[i]);
        i++;
        nodes_visited++;
    }
    return j;
}

template<typename T>
// let's assume these are contiguous
void merge(T * A, size_t sizeA, T * B, size_t sizeB) {
    size_t swap_space_size = 0;
    T * swap_space = B;
    size_t B_index = leftMerge(A, sizeA, B, sizeB, swap_space, swap_space_size);
    bool first_run = true;
    // then again. if swap_space isn't empty merge swap_space with remaining B
    while (first_run || swap_space_size > 1) {
        first_run = false;

        A = swap_space;
        sizeA = swap_space_size;

        // hmmm... I think that's it...

        // so, typically, swap_space == B but if swap_space won at some point AND then extended and B won, 
        //  then there's a gap between end of swap_space and start of unsorted elements...

        // now, think about how repeating over and over is affected by this...
        // we'll iterate over the new swap_space... 

        // hmm... I think this might be it...

        B = &B[B_index];
        swap_space = B;
        sizeB -= B_index;
        swap_space_size = 0;
        leftMerge(A, sizeA, B, sizeB, swap_space, swap_space_size);

        // okay, so repeat until single element in swap_space... hmm... this is increasingly more dumb lol..
        //  inherent time/space trade-off?
    }
    // if odd size before split,
    // j may not equal sizeB but that's fine because it should already be sorted... 

    // Yeah, so now the question is whether these extra operations are worse than allocating a new array...
    //  probably splitting hairs (hares?) lol - but interesting...
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