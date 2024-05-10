#include <stdint.h>
#include <stdio.h>

#include "logger.h"

#ifndef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_TEST
#endif

int main() {
    // uint16_t size = 65536 / 4;
    uint16_t size = 16 / 4;
    logger_printf(LOGGER_TEST, "size: %u\n", size);
    for (int i = 0; i < size + 1; i++) {
        logger_printf(LOGGER_TEST, "\n*********************iteration: %d\n", i);
        // TODO: "type"-new function seg faults when malloc returns NULL... 
        int * ptr = new int(1);
        //  (I assume it does some additional casting... and such but you would think they thought of that? LMAO) 
        // int * ptr = (int *) ::operator new(sizeof(int));
        if (ptr == NULL) {
            logger_printf(LOGGER_TEST, "NULL POINTER!\n");
        }
    }
    return 0;
}