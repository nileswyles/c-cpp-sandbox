#include "stdio.h"

// I don't expect some of these to work lol... 
int return_value_arg_ref(int& i) {
    return i;
}

int& return_ref() {
    int i = 7;
    return i;
}

int& return_ref_arg_ref(int& i) {
    return i;
}

int& return_ref_arg_val(int i) {
    return i;
}

int * return_ptr_arg_ref(int& i) {
    return &i;
}

int main() {
    // Most interesting...
    int arg_ref_3 = 7;
    int * ret_ptr = return_ptr_arg_ref(arg_ref_3);
    printf("return_ptr_arg_ref: %d, %p, %p\n", *ret_ptr, ret_ptr, &arg_ref_3);
    // lmaooo

    // these are valid...
    int arg_ref_1 = 7;
    printf("return_value_arg_ref: %d\n", return_value_arg_ref(arg_ref_1));
    // that said, you probably will never want to do this...
    int arg_ref_2 = 7;
    printf("return_ref_arg_ref: %d\n", return_ref_arg_ref(arg_ref_2));
    // but can use that function to test this...
    int& ret_ref = return_ref_arg_ref(arg_ref_2);
    printf("reference bridge?: %d, %p, %p\n", ret_ref, &ret_ref, &arg_ref_2);
    // and so, that begs the question... do you need the & designator?
    int ret_ref_2 = return_ref_arg_ref(arg_ref_2);
    printf("reference bridge?: %d, %p, %p\n", ret_ref_2, &ret_ref_2, &arg_ref_2);

    // now is this defined in the spec? or undefined/impl based? lol...

    // these segfault...
    printf("return_ref_arg_val: %d\n", return_ref_arg_val(7));
    printf("return_ref: %d\n", return_ref());

    return 1;
}