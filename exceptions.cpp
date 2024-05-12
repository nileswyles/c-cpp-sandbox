#include <iostream>
#include <stdexcept>

// might need to nest and non-primitive types?

// also, doesn't require some trap registers (or similar mechanism) in the case of an embedded system?

//  exception object created that requires dynamic storage duration (new)
//  std::terminate called if exception thrown during exception handling... lol... by C++ runtime? Whatever that is? type reflection, stack management, etc?
//      - std::terminate_handler

// see noexcept


int function(int i, int e) {
    int x = i + 5;

    if (i == 1) {
        throw std::runtime_error("error"); 
    }

    return x + e * 7;
}

int main() {
    try {
        function(2, 7);
        function(1, 9);
    } catch (const std::exception& e) {
        std::cout << e.what() << '\n';
        //  throw e; // copy-initializes a new exception object of type std::exception
        // throw;   // rethrows the exception object of type std::out_of_range
    }

    return 0;
}