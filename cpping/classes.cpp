#include "stdio.h"

class Test {
    public:
        size_t i;
        // that is definetly valid cpp....?
        // Test::Test(size_t i) : i(i) {
        Test(size_t i) : i(i) {

        } 

        // Why do I need to define this?
        Test() : i(1) {}
        // Test::Test() : i(0) {}

        // lol... 
        static Test testConstructor(size_t i) {
            return Test(i);
        } 
};

int main() {
    Test t(7);
    Test t_constructor = Test(7);
    Test t_constructor_default = Test();
    Test t_constructor_func_static = Test::testConstructor(7);
    printf("%lu\n", t.i);
    printf("%lu\n", t_constructor.i);
    printf("%lu\n", t_constructor_default.i);
    printf("%lu\n", t_constructor_func_static.i);

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