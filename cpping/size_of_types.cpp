#include <string>
#include <unordered_map>

class Test {
    public:
        uint8_t i;
};

class TestWFunction {
    public:
        uint8_t i;
        TestWFunction(int i): i(i) {}
    
        void testFunction() {
            i++;
        }
};

int main() {
    printf("Test %d\n", sizeof(Test));
    printf("TestWFunction %d\n", sizeof(TestWFunction));
    printf("std::string %d\n", sizeof(std::string));
    printf("unordered_map<string, string> %d", sizeof(std::unordered_map<std::string, std::string>));
    printf("\n");
    return 0;
}