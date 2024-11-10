
#include <iostream>
#include "string-format.h"

// testFormat
// testFormatInt
//  

int main(int argc, char * argv[]) {
    std::cout << "DOES IT BLEND?\n";

    std::cout << WylesLibs::format("Test {}\n", "lol");

    return 0;
}