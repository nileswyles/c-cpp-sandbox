#include "stdio.h"

class Test2 {
    public:
        size_t i;
        size_t size;

    Test2() : i(7) {
        printf("DEFAULT CONSTRUCTOR CALLED\n");
    }

    Test2(size_t i) : i(i) {}
};

class Test {
    public:
        size_t i;
        Test2 testStruct;
        // Test::Test(size_t i) : i(i) {
        Test(size_t i) : i(i) {
            // So this tells me storage for variables is allocated and initialized before this constructor is called... default consstructor is also called?
            // makes sense, and what I thought but good to confirm...?
            printf("Address of %p, i: %lu, size: %lu\n", &testStruct, testStruct.i, testStruct.size);
            testStruct.i = 4;
            printf("Address of %p, i: %lu, size: %lu\n", &testStruct, testStruct.i, testStruct.size);
            testStruct = Test2(1);
            printf("Address of %p, i: %lu, size: %lu\n", &testStruct, testStruct.i, testStruct.size);
        } 

        Test() : i(1) {}
        static Test testConstructor(size_t i) {
            return Test(i);
        } 
};

typedef void(FunctionPtr)();
class TestFuncPtr {
    public:
        FunctionPtr * func;
        void functionValue() {
            printf("test\n");
        }
        // TestFuncPtr() {
        //     func = functionValue;
        //     func();
        // }
        TestFuncPtr() {}
        TestFuncPtr(FunctionPtr * func): func(func) {
            // functionValue = func;
            func();
        }
};

int main() {
    // TestFuncPtr testFuncPtr();
    TestFuncPtr testFuncPtr;
    // why does it have to be static?
    TestFuncPtr testFuncPtr2(testFuncPtr.functionValue);

    // Test t(7);
    // Test t_constructor = Test(7);
    // Test t_constructor_default = Test();
    // Test t_constructor_func_static = Test::testConstructor(7);
    // printf("%lu\n", t.i);
    // printf("%lu\n", t_constructor.i);
    // printf("%lu\n", t_constructor_default.i);
    // printf("%lu\n", t_constructor_func_static.i);

    // moral of the story... cpp defines "concepts" that don't fall in typical categories of function/variable/class? 
    //  lol... I guess constructor is the category for Test()? 

    // so...

    // you can call constructor directly? 
    //      <type-name>()
    // but also via other type of expresions... 
    //      <type-name> <variable-name>[()]; 
    //      new <type-name>;

    // hmm....
    // interesting observation

    return 1;
}