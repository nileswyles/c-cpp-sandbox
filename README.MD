# Philosophy, (my)best practices
-- BE EXPLICIT (especially in header files)
-- remember to extern, const (but maybe don't over use?), static, inline and ?restrict?
-- explicitly cast/convert types, because g++ (among other things)...
-- define static functions and variables at top of file...
-- when function needs to report error codes, return as value so that we can do 'if(routine() == 1) blah' 
-- GNU compiler flags... -Wall, and actually make sure to address warnings lol. Can optionally make them errors using -Wpedantic... This one seems useful... -Wincompatible-pointer-types
-- Where possible, declare and allocate memory for variables at top of function block, to more easily (visually) check that they are freed.

-- Are there any cool optimization related tips and tricks?
-- Weak symbols for testing? GCC -nostdlib? Apparently, syscalls are (functionally) weak symbols - I was able to stub the read call without any compiler/linker FOO. 

TODO: Read about more best practices
https://en.cppreference.com/w/c/keyword
https://en.cppreference.com/w/c/language/storage_duration
https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
