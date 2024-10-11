#include "stdio.h"

class Base {
    public:
        int lol;
        Base() {
            printf("\tBase default constructor\n");
        }
        Base(int lol): lol(lol) {
            printf("\tBase non-default constructor: %d\n", lol);
        }
        virtual ~Base() {
            printf("\tBase destructor\n");
        }
        virtual void testConstructor() {} 
        Base operator= (const Base& other) {
            printf("\tBase copy assignment called.\n");
            return *this;
        }
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
        Test(const Test& other): Base(27) {
            printf("\tTest copy constructor called.\n");
        }
        Test& operator= (const Test& other) {
            printf("\tTest copy assignment called. this->i = %d, other->i = %d\n", this->i, other.i);
            return *this;
        }
};

// most recent output:
// Stack variable of Test class
//         Base default constructor
//         Test default constructor


// Heap variable of Test class
//         Base default constructor
//         Test default constructor


// Heap variable of Test(27) class
//         Base default constructor
//         Test non-default constructor: 27


// Move assignment
//         Base default constructor
//         Test default constructor


// Copy Constructor
//         Base non-default constructor: 27
//         Test copy constructor called.


// Copy assignment
//         Test copy assignment called. this->i = 65535, other->i = 27


// Heap variable of Base class
//         Base default constructor


// Deleting Heap variable of Test class
//         Test destructor
//         Base destructor


// Deleting Heap variable of Test(27) class
//         Test destructor
//         Base destructor


// Deleting Heap variable of Base class
//         Base destructor


// Stack variables should be deleted
//         Test destructor
//         Base destructor
//         Test destructor
//         Base destructor
//         Test destructor
//         Base destructor

// findings:
//  - no, compiler error saying there isn't a default constructor? lol, that's oddd.. 
//      so, it either implements a default or derived classes don't implicitly call the constructor...

//  - Base constructor and destructor are indeed called regardless of how it's initialized... 
//      - default if not explicitly called..
//  - This means, the default isn't required... lol, as I thought?
//  - Base assignment operator is not called implicitly...
int main() {
    printf("Stack variable of Test class\n");
    Test t;
    printf("\n\nHeap variable of Test class\n");
    Test * test = new Test; 
    printf("\n\nHeap variable of Test(27) class\n");
    Test * test_27 = new Test(27); 
    printf("\n\nMove assignment\n");
    Test move = Test();
    printf("\n\nCopy Constructor\n");
    Test copy = t;
    printf("\n\nCopy assignment\n");
    copy = *test_27;
    printf("\n\nHeap variable of Base class\n");
    Base * test2 = new Base; 
    printf("\n\nDeleting Heap variable of Test class\n");
    delete test;
    printf("\n\nDeleting Heap variable of Test(27) class\n");
    delete test_27;
    printf("\n\nDeleting Heap variable of Base class\n");
    delete test2;

    printf("\n\nStack variables should be deleted\n");
    return 1;
}