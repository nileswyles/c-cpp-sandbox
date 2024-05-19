#include "stdio.h"
#include <vector>

void vectors(std::vector<int> test) {
    test.push_back(77);
}

std::vector<int> vector_test(std::vector<int> test) {
    std::vector<int> vector_local = test;

    test.push_back(177);

    vector_local.push_back(177777);

    printf("test: %d, vector_local: %d, %p, this is obviously copied right?: %p\n", test[1], vector_local[1], vector_local.data(), vector_local);
    vectors(test);

    return vector_local;
}

// hmmm... yeah might not be a good idea anyways? because who knows sizeof(vector?) sizeof(vector *) is predictable?
//      time to refactor the parser... lol...
//      or just assume it's taken care of?
//      how does reference apply here? address of should match original scope?

//  interesting.....

class TestClass {
    public:
        int * i;
        TestClass() {
            i = new int(7);
            printf("Constructor called.\n");
        }
        ~TestClass() {
            printf("Deconstructor called., %p\n", this);
        }
        // move and copy do the same in this example...
        // copy... 
        TestClass(const TestClass& x) : i(x.i) {
            printf("Copy constructor called.\n");
        } // lol...
        TestClass& operator= (const TestClass& x) {
            printf("Copy assignment called.\n");
            this->i = x.i;
            // lol.. this seems like not the right way to do this?
            //  let's find out...
            // this->cap = x.cap();
            // this->size = x.getSize();
            return *this;
        }
        // Move
        TestClass(TestClass&& x) : i(x.i) {
            printf("Move constructor called.\n");
        }
        TestClass& operator= (TestClass&& x) {
            printf("Move assignment called.\n");
            this->i = x.i;
            return *this;
        }
        // // Move constructor
        // T (Example6&& x) : ptr(x.ptr) {
        //     x.ptr=nullptr;
        // }
        // // Move assignment
        // T& operator= (T&& x) {
        //   delete ptr; 
        //   ptr = x.ptr;
        //   x.ptr=nullptr;
        //   return *this;
        // }
};

TestClass testFunction2(TestClass testClass) {
    printf("testClass 2: %p, %p\n", &testClass, testClass.i);
    TestClass ret = testClass;
    *(ret.i) = 77; 
    printf("ret testClass: %p, %p\n", &ret, ret.i);
    return ret;
}

void testFunction(TestClass testClass) {
    printf("testClass: %p, %p\n", &testClass, testClass.i);
    TestClass ret = testFunction2(testClass);
    printf("ret testClass 2: %p, %p\n", &ret, ret.i);
}

int main() {
    TestClass t;
    testFunction(t);


    // std::vector<int> t;
    // t.push_back(7);

    // std::vector<int> blahblahblahblahabl = vector_test(t);

    // for (auto val: t) {
    //     // hmm.... lol that doesn't make much sense...
    //     printf("%d\n", val);
    // }

    // printf("returned vector local ptr: %p, %p\n", blahblahblahblahabl.data(), &blahblahblahblahabl);

    // for (auto val: blahblahblahblahabl) {
    //     // hmm.... lol that doesn't make much sense...
    //     printf("%d\n", val);
    // }

    return 1;
}