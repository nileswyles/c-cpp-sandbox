#pragma once

#if defined(_MSC_VER)

// Microsoft seems to suggest the "deprecated" functions aren't secure - I'll just have to take their word for it.
#define strcat strcat_s
// #define strcpy strcpy_s
#define sprintf sprintf_s

#endif