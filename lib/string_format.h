#ifndef WYLESLIBS_STRING_FORMAT_H
#define WYLESLIBS_STRING_FORMAT_H

#include <string>

namespace WylesLibs {
// alright, so stepping back this is a parser and we can build the datastructure (string) as we parse for optimal performance.
//  learning from json parser experience... I think I can benefit from top-down thinking here... 

//  let's map the tree
//  format
//      -> parse_non_format (just appends character to string datastructure)
//      -> parse_format
//          -> parse_numeric
//          -> parse_type_specifier

//
// var-array of args...

// { == start_format_specifier
//  } == stop_format_specifier
//  TBD if that will need to change...

// parse_format -> found_start_of_specifier (and peek ahead?)
//   if isNumeric, then it's position reference, so allows us to explicitly select the position.
//      should be followed by comma or stop_format_specifier increment cursor accordingly
//      parse_numeric
//   if isTypeSpecifier
//      should be followed by comma or stop_format_specifier increment cursor accordingly
//      parse_type_specifier
//   done parsing format

// yeah, so, should I use string and cursor or stringstream?
// too much overhead from stringstream? ddd. stream from IStreamEStream... right? lol that's the whole point...

// let's be crude about this...
#define WYLESLIBS_STRING_FORMAT_DEFAULT_ARG_TYPE_CHAR '|'
class Arg {
    public:
        void * ptr;
        std::string expanded_value;
        char type;
        Arg(): ptr(nullptr), type(WYLESLIBS_STRING_FORMAT_DEFAULT_ARG_TYPE_CHAR), expanded_value("") {}
        Arg(void * ptr, char type, std::string expanded_value): ptr(ptr), type(type), expanded_value(expanded_value) {}
        virtual ~Arg() {}
};

extern std::string format(std::string format, ...);
// LMAO
#define MAX_FORMATTED_STRING_SIZE 1024
extern std::string ssprintf(std::string format, ...);
};
#endif