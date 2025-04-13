#ifndef WYLESLIBS_JSTRING_H
#define WYLESLIBS_JSTRING_H

#if defined(JSON_STRING_PLATFORM_STRING_UWP_CX)
#include "pch.h"
#else 
#include <string>
#endif

namespace WylesLibs::Parser::Json {
#if defined(JSON_STRING_U32)
typedef std::u32string jstring;
typedef u32char_t jchar;
#elif defined(JSON_STRING_U16)
// TODO: what's the difference between U16 and WSTRING? Endianess? likely nothing? because I think endianess is system dependent?
typedef std::u16string jstring;
typedef u16char_t jchar;
#elif defined(JSON_STRING_PLATFORM_STRING_UWP_CX)
// TODO: Implicit conversion, right? lol... otherwise, #define?
typedef Platform::String^ jstring;
typedef wchar_t jchar;
//#define jstring Platform::String^;
#elif defined(JSON_STRING_WSTRING)
typedef std::wstring jstring;
typedef wchar_t jchar;
#else
typedef std::string jstring;
typedef char jchar;
#endif
};

#endif 