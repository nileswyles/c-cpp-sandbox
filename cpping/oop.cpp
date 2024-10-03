#include "stdio.h"

class BaseAbstract {
    public:
        // void test() {
        //     printf("BaseAbstract test()\n");
        // }
        // virtual void test() {
        //     printf("BaseAbstract test()\n");
        // }
        virtual void test() = 0;
};

class Test: public BaseAbstract {
    public:
        Test() {}

        void test() {
            printf("Test test()\n");
        }
};

int main() {
    Test t;
    t.test();
    BaseAbstract * testAbstractPtr = &t;
    testAbstractPtr->test();
    return 1;
}