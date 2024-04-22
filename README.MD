# Philosophy, (my)best practices
-- BE EXPLICIT (especially in header files)
-- remember to extern, const (i.e, string literals), static, inline and ?restrict?
-- explicitly cast/convert types, because g++ (among other things)...
-- declare static functions at top of file, define at bottom - because order matters, so if static function calls another static function it can get annoying.
-- define static variables at top of file...
-- when function needs to report error codes, return as value so that we can do 'if(routine() == 1) blah' 
-- GNU compiler flags... -Wall, and actually make sure to address warnings lol. Can optionally make them errors using -Wpedantic... This one seems useful... -Wincompatible-pointer-types
-- Where possible, declare and allocate memory for variables at top of function block, to more easily (visually) check that they are freed.

-- Are there any cool optimization related tips and tricks?
-- Weak symbols for testing? GCC -nostdlib? Apparently, syscalls are (functionally) weak symbols - I was able to stub the read call without any compiler/linker FOO. 
-- Macros

- Sort of decided to use g++ for everything but why?
    - Should I implement shared_ptr, string and other abstractions from the standard library? (Still yes?)
    - Do templates/generics provide any value? Classes, polymorphism or inheritance? 
        - Can you generally structure programs in a simpler way instead of leveraging these language features? 

    - I think they do, so while we're at it - use namespaces too.
    - It's about Options... Options... Options...

TODO: Read about more best practices
https://en.cppreference.com/w/c/keyword
https://en.cppreference.com/w/c/language/storage_duration
https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html
https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html