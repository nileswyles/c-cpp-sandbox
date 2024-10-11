#include "stdio.h"

class Base {
    public:
        Base() {
            printf("\tBase default constructor\n");
        }
        virtual ~Base() {
            printf("\tBase destructor\n");
        }
        virtual void testConstructor() {} 
};

class Test: public Base {
    public:
        int i;
        Test() {
            printf("\tTest default constructor\n");
        }
        Test(int i): i(i) {
            printf("\tTest non-default constructor: %d\n", i);
        }
        ~Test() {
            printf("\tTest destructor\n");
        }
        void testConstructor() {} 
};

// findings:
//  - no, compiler error saying there isn't a default constructor? lol, that's oddd.. 
//      so, it either implements a default or derived classes don't implicitly call the constructor...

//  - Base constructor and destructor are indeed called regardless of how it's initialized... 
//  - This means, the default isn't required... lol, as I thought?
int main() {
    printf("Stack variable of Test class\n");
    Test t;
    printf("\n\nHeap variable of Test class\n");
    Test * test = new Test; 
    printf("\n\nHeap variable of Test(27) class\n");
    Test * test_27 = new Test(27); 
    printf("\n\nHeap variable of Base class\n");
    Base * test2 = new Base; 
    printf("\n\nDeleting Heap variable of Test class\n");
    delete test;
    printf("\n\nDeleting Heap variable of Test(27) class\n");
    delete test_27;
    printf("\n\nDeleting Heap variable of Base class\n");
    delete test2;

    printf("\n\nStack variable should be deleted\n");
    return 1;
}