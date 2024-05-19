# Philosophy, (my)best practices
- BE EXPLICIT (especially in header files)
    - On second thought, how explicit do we want to be? Specifically regarding CPP class defintions, don't think I want to explicitly define all operators, constructors, etc... (YOU ARE THE ONLY EXCEPTION ---_ YOUUUUUUU ARE THE ONLY EXCEPTION LMAO)
    - Definitely still explicitly use extern and static in C.  
- remember to extern, const (i.e, string literals), static, inline and ?restrict?
    - functions extern by default.
    - variables static by default.
        - It appears in C++ context those keywords only apply to "naked" functions and variables? 
    - Global variables are generally bad lol, but static functions are common?, especially when implemented in header files? (another code smell?, seems useful to me.)
- declare static functions at top of file, define at bottom - because order matters, so if static function calls another static function it can get annoying.
- define static variables at top of file...
- explicitly cast/convert types, because g++ (among other things)...
    - TODO: use of cpp cast wrappers?
- when function needs to report error codes, return as value so that we can do 'if(routine() == 1) blah' 
- GNU compiler flags... -Wall, and actually make sure to address warnings lol. Can optionally make them errors using -Wpedantic... This one seems useful... -Wincompatible-pointer-types
- Where possible, declare and allocate memory for variables at top of function block, to more easily (visually) check that they are freed.

- Are there any cool optimization related tips and tricks?
- Weak symbols for testing? GCC -nostdlib? Apparently, syscalls are (functionally) weak symbols - I was able to stub the read call without any compiler/linker FOO. 
- Macros
- In an effort to keep things simple (but at the same time, in a round about way, make things more complicated) and because it's more fun, don't use the standard lib... let's write our own.

- Sort of decided to use g++ for everything, but why?
    - Draw clear distinction between C and CPP code. 
    - Should I implement shared_ptr, string and other abstractions from the standard library? (Still yes?)
    - Do templates/generics provide any value? Classes, polymorphism or inheritance? 
        - Can you generally structure programs in a simpler way instead of leveraging these language features? 
    - I think they do, so while we're at it - use namespaces too.
    - It's about Options... Options... Options...

- Thoughts on malloc? Pass by reference instead? Then no need to free? Can't completely avoid it though? Like for dynamically sized data structures?
- Also, testing is important. Limiting exposure to bugs among and above? other things.

# Library Details
- Interfaces with POSIX. Win32 as well, eventually - why either or? Should be easy to extend. 
- API will not be very thought out at first but should improve over time.

TODO: Read about best practices
https://google.github.io/styleguide/cppguide.html
https://en.cppreference.com/w/c/keyword
https://en.cppreference.com/w/c/language/storage_duration
https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html
https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html

- Statically allocated data structures for embedded systems with no real dynamic memory allocation mechanism, or where consistent, predictable performance is really important? 
    - Ring buffer..
        - Heap
        - Binary Tree

    - CPP provide any value there?
    -   RAII?
    - HMM... might want to bring back the C array implementation lol and reader too? 
        Which might have been the original plan? embedded stuff typically c and isolated but cpp might be cleaner
        Or write allocator/redefine new

        Global replacements
        The versions (1-4) are implicitly declared in each translation unit even if the <new> header is not included. Versions (1-8) are replaceable: a user-provided non-member function with the same signature defined anywhere in the program, in any source file, replaces the default version. Its declaration does not need to be visible.


# Plan for data structures? At least for now...
Let's think about this some more before proceeding? Is iterator abstraction really necessary?
     What "Collection" types do I plan to implement? 
         - Array
         - Linked List
         - Priority List (Heap)
         - Map
         - Static Array (RingBuffer/Index), can also share interface...?

     Now let's explore how you would invoke each of the afformentioned types? 

     LMAO, yeah... not necessary to go further... definetly want to abstract (generalize) here ;)... because DRY.

     
     So we typically iterate want to:
         hmm.... so this basically takes any data structure and makes it an array... dun dun dun.... because sequential lol... 
         and so, we might want to iterate forwards, backwards and starting from any index in the abstracted array...

     So, if that's the case, then can we just encode the structures directly in the array type I already implemented?
     no... because pointer maths allow for faster algo's... okay... yeah... abstraction it is...

     right? because making sure contigious is more computation... yeah... 

     additionally, Array is really only structure where you might want to expose the underlying data.

     Let's copy CPP's STL interface because why not?

In conclusion... good exercises... Let's prioritize statically sized datastructures for when we don't want or cannot use malloc/new...
     Static Array/Ring buffer, FIFO, STACK, SORTED LIST... then build priority lists 

Let's implement JSON using STL for now... lol

    alright, to justify/solidify/(and so I don't forget) my reasoning... abstractions generally incur overhead... 
         The goal here is to minimize that overhead when necessary (when optimizing for performance). That was the journey I embarked on originally and lost my way? lol

    and so, instead of running straight to some higher-level abstraction... Let's only generalize only as much as it doesn't affect performance.
        or rather specialize as much as possible... 

    Even if it's a backwards way of thinking? 

    More brain dump, retrospective, reflection, reminder, affermation:
        In other words, let's fully understand the problem, to the very finest detail (at least as much as possible). Then make optimizations/generalize where possible. Or have the foresight (executive planning?), (which also might be the same thing?), just a faster way of arriving there? Yeah, I think that's the right way of going about it. 

        more generally, "bring your whole set", consider all possible applications of what you're implementing. Idk, maybe I'm wrong... 

    i.e. Containers copy/assignment incurs overhead? (bad example? maybe...)
         a = b	C&	Destroys or copy-assigns all elements of a from elements of b

So there are definitely situations where I might want to use my Array class over vector, for example. But might not make sense to reimplement the higher level abstractions.

To expand further... minimum overhead for "node"-based data structures is size of pointer... And I am not sure but there might be more... like the iterator stuff?
    or does the compiler work it's magic?