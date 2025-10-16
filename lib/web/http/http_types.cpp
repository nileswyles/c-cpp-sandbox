#include "http_types.h"

using namespace WylesLibs::Http;

//  Only because there's ambiguity or gap in knowledge or misalignment?
//      Let's eliminate ambiguity in the program by moving requestMap initialization from main.cpp to http_types.cpp (at least until I further investigate).

//      Decoupling eliminates the potential spagetti/entanglement between example.cpp in addition to simplifying the compiler's work?
//      The above is why you "generally" want to avoid global variables.
//      Is there a compiler warning for this (other than segfault)? Specifically, in the event you require some static global from where this is modified or parallels might be a thing - I think? so translation unit abstraction is only useful when it's useful... 
//      idk, on second thought, might be heavy-handed for something that is verifiable.

//      the concern being whether the main translation unit needs to be last (or more specifically after controllers/*) for some implementation specific reason. Optimizing for portability?
//      otherwise, I 'think' the globally accepted definition of the language - wrt to the translation unit/dependency graph/static - provides all facilities for this to work.

// initialize requestMap --- see http_types.h and controllers/example.cpp for more details.
SharedArray<HttpProcessorItem> WylesLibs::Http::requestMap = SharedArray<HttpProcessorItem>();