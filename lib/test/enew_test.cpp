#include <stdint.h>
#include <stdio.h>

int main() {
    uint16_t size = 65536 / 4;
    printf("size: %u\n", size);
    for (int i = 0; i < size + 1; i++) {
        printf("iteration: %d\n", i);
        int * ptr = new int(1);
        if (ptr == NULL) {
            printf("NULL POINTER!\n");
        }
    }
    return 0;
}