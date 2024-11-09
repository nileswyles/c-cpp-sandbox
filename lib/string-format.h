#ifndef WYLESLIBS_STRING_FORMAT_H
#define WYLESLIBS_STRING_FORMAT_H

#include <string>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>

#define format(format, ...) \
    if () { \ 
        const 
    }

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
//   if isNumeric, then it's position indicator, so allows us to explicitly select the position.
//      should be followed by comma or stop_format_specifier increment cursor accordingly
//      parse_numeric
//   if isTypeSpecifier
//      should be followed by comma or stop_format_specifier increment cursor accordingly
//      parse_type_specifier
//   done parsing format

// yeah, so, should I use string and cursor or stringstream?
// too much overhead from stringstream? ddd. stream from readerestream... right? lol that's the whole point...

// let's be crude about this...
class Arg {
    public:
        void * ptr;
        std::string expanded_value;
        char type;
        Arg(void * ptr, char type, std::string expanded_value): ptr(ptr), type(type), expanded_value {}
        virtual ~Arg() {}
};

#define MAX_FORMAT_LENGTH 1024;
#define START_OF_FORMAT_CHAR '{'
#define END_OF_FORMAT_CHAR '}'

static const std::string TYPE_SPECIFIER_MATCH("dbsft");

static Arg parsePositionalPercentFormat(EStream * s) {
    std::string v;
    void * ptr = nullptr;
    char type;

    char start_char = s.get();
    SharedArray<uint8_t> f = s->readUntil(END_OF_FORMAT_CHAR, false);
    char * format = f.toString().c_str();
    char c = f.back(); // last char

    char expanded_value[MAX_FORMAT_LENGTH];
    // no G or A because wtf even is that?
    if (c == start_char) {
        if (c == 'b') {
            bool b = va_args(args, bool);
            ptr = (void *)new bool(b);
            type = 'b';
            if (true == b) {
                v += "true";
            } else {
                v += "false";
            }
        } else if (c == 't') {
            std::ios_base type_stream = va_arg(args, std::ios_base&);
            ptr = (void *)new type_stream(b);
            type = 't';
            v << type_stream;
        } else if (c == 'c' && c == start_char){
            char i = va_args(args, char);
            ptr = (void *)new char(i);
            sprintf(expanded_value, format, i);
            type = 'c';
            v = expanded_value;
        } else if (c == 's' && c == start_char) {
            v = va_args(args, std::string);
            type = 's';
        } else {
            // some error?
        }
    } else {
        if (c == 'd' || c == 'x' || c == 'X' || c == 'o') {
            int i = va_args(args, int);
            ptr = (void *)new int(i);
            type = 'd';
            sprintf(expanded_value, format, i);
            v = expanded_value;
        } else if (c == 'f' || c == 'e' || c == 'E') {
            double f = va_args(args, double);
            ptr = (void *)new double(f);
            type = 'f';
            sprintf(expanded_value, format, f);
            v = expanded_value;
        } else {
            // some error?
        }
    }
    return Arg(ptr, type, v);
}

static std::string parseIndicatorPercentFormat(EStream * s, Arg arg) {
    std::string v;
    char start_char = s.get();
    SharedArray<uint8_t> f = s->readUntil(END_OF_FORMAT_CHAR, false);
    char * format = f.toString().c_str();
    char c = f.back(); // last char

    char expanded_value[MAX_FORMAT_LENGTH];
    if (c == start_char) {
        if (c == 'b') {
            v += arg.expanded_value;
        } else if (c == 't') {
            v += arg.expanded_value;
        } else if (c == 's') {
            v = arg.expanded_value;
        } else if (c == 'c' && c == start_char){
            sprintf(expanded_value, format, *((char *)arg.ptr));
            v = expanded_value;
        } else {
            // some error?
        }
    } else {
        // no G or A because wtf even is that?
        if (c == 'd' || c == 'x' || c == 'X' || c == 'o') {
            sprintf(expanded_value, format, *((int *)arg.ptr));
            v = expanded_value;
        } else if (c == 'f' || c == 'e' || c == 'E') {
            sprintf(expanded_value, format, *((double *)arg.ptr));
            v = expanded_value;
        } else {
            // some error?
        }
    }
    return v;
}

// TODO:
// escape characters...
//  {} and any required by printf? if any?
//  if don't need to escape characters for printf then maybe choose different start/end characters?

static void parsePositionalFormat(va_list args, EStream * s, std::stringstream& d, Array<Arg>& converted_args, bool& has_indicator_selection) {
    // i == {
    char c = s.get();
    Arg arg;
    // order of number, type_specifier matters... we only support {i,t} not {t,i}
    if (c == isNumber) {
        // it's an indicator type, reserve parsing for later.
        c = s.get();
        while(c != END_OF_FORMAT_CHAR) {
            d.put(c);
            c = s.get();
        }
        return;
    } 

    c = s.get();
    if (c == END_OF_FORMAT_CHAR) {
        Arg arg;
        arg.expanded_value = va_args(args, std::string);
        type = 's';
    } else if (c == '%') {
        arg = parsePositionalPercentFormat(s);
    } else {
        // some error?
    }
    d.write(arg.expanded_value.data(), arg.expanded_value.size());
    converted_args.append(arg);
    // i == }
}

static void parseIndicatorFormat(Array<Arg>& args, EStream * s, std::stringstream& d, size_t& i) {
    // i == {
    size_t selection = SIZE_MAX;
    char c = s.get();
    if (c == isNumber) {
        // parse number...
        double lol = 0;
        s->readNatural(lol);
        selection = static_cast<size_t>(lol) - 1; // 0 - 1 == SIZE_MAX?
    }
    if (selection == SIZE_MAX || selection >= args.size()) {
        d += "";
        // or throw exception? what does that even mean in this context? lol..
    } else {
        std::string v;
        Arg arg = args[selection];
        c = s.get();
        if (c == END_OF_FORMAT_CHAR) {
            v = arg.expanded_value();
        } else if (c == ',') {
            c = s.get();
            if (c == '%') { // else must be type specifier?
                v = parseIndicatorPercentFormat(s, arg);
            } else {
                // throw error
            }
        } else {
            // throw error
        }
        d.seekg(i);
        d.write(v.data(), v.size());
        i += s.size();
    }
    // } // should never see empty format block here
    // i == }
}

static std::string format(std::string format, ...) {
    va_list args;
    va_start(args, format);

    // TODO: maybe don't need this? just append to args if needed? and check size? hmm.. that might not be right..

    // if reach end of format and still va_args, then do what? can just ignore if just positional but might be an issue with indicator stuff..
    bool has_indicator_selection = false;
    std::stringstream d;
    Array<Arg> parsed_args;
    EStream s(format.data(), format.size());
    char c = s.get();
    while (c) {
        s.get();
        if (c == '{') {
            parsePositionalFormat(args, &s, d, parsed_args, has_indicator_selection);
        } else {
            d.put(c);
        }
        c = s.peek();
    }

    // stringstream to EStream? hmm...
    EStream s_w_parsing(d.data(), d.size());
    if (true == has_indicator_selection) {
        size_t i = 0;
        while (c) {
            s_w_parsing.get();
            if (c == '{') {
                parseIndicatorFormat(parsed_args, &s_w_parsing, d, i);
            }
            // at least one character is always read.
            i++;
            c = s_w_parsing.peek();
        }
    }
    va_end(args);
}

// static std::string format(std::string format, ...) {
static std::string sprintf(std::string format, __gnuc_va_list __ap) {
    // va_list args;
    // va_start(args, format);

    char lol[1024];
    // sprintf(lol, format, args);
    sprintf(lol, format, __ap);

    // va_end(args);

    return std::string(lol);
}
}
#endif