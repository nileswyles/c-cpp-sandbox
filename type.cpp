#include <stdint.h>
#include <stdio.h>

int main() {

    // uint64_t saturation = UINT64_MAX;
    // uint64_t saturation = 77777777770;

    // uint64_t saturation = 9999;
    uint64_t saturation = 9999;
    // uint16_t test = static_cast<uint16_t>(saturation);
    // uint16_t test = (uint16_t)saturation;
    uint16_t test = uint16_t(saturation);

    printf("lol: %u, %x, %lx\n", test, test, saturation);

    return 0;
}