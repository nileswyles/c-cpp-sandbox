
#include "stdio.h"
#include <initializer_list>

typedef int* const_iterator;

class Test {
    public:
        int * i;
        int size;
        Test() {
            size = 4;
            i = new int[size];
            for (int y = 0; y < size; y++) {
                i[y] = y + 17;
            }
        };
        ~Test() = default;
        int * begin() {
            return i;
        }
        int * end() {
            return i + size;
        }
};

// g++ cpping/iterators.cpp -Wall -o iterators.out
int main() {
    int size = 4;
    int * t = new int[size];
    t[0] = 0;
    t[1] = 1;
    t[2] = 2;
    t[3] = 3;

    const_iterator start = t;
    const_iterator end = t + size;

    // just confirming pointer arithmetic still works after obfuscation, but obviously? lol
    int * t_ptr_maths_q = start + 1;
    printf("lol: %d\n", *t_ptr_maths_q);

    for (int i = 0; i < size; i++) {
        printf("%d ", t[i]);
    }

    printf("\n");

    // iteration_list of ints...?
    // auto&& iterable_collection = {0, 1, 3, 4};
    // auto&& iterable_collection{0, 1, 3, 4};
    // std::initializer_list<int> iterable_collection = {0, 1, 3, 4};
    std::initializer_list<int> iterable_collection{0, 1, 3, 4};
    for (auto i: iterable_collection) {
        printf("%d ", i);
    }

    printf("\n");

    for (auto i: {6, 7, 8, 9}) {
        printf("%d ", i);
    }

    printf("\n");

    // so, similarly, just defining begin and end in a class makes it iterable - automagically?
    Test iterable_type;
    for (auto i: iterable_type) {
        printf("%d ", i);
    }

    printf("\n");

    return 0;
}