#include "stdio.h"

int main() {
    double test = 272727272.2028018;
    for (size_t i = 0; i < 128; i++) {
        test *= 10;
        printf("%4.2f %+.0e %E\n", test, test, test);
    }
    double test2 = 2772727272.19029019;
    for (size_t i = 0; i < 128; i++) {
        test2 = test2/10;
        printf("%f\n", test2);
    }

    return 1;
}