#ifndef WYLESLIBS_JSON_MAPPER_H
#define WYLESLIBS_JSON_MAPPER_H

#include "parser/json_parser.h"
#include "parser/json_object.h"
#include "parser/json_array.h"

#include "logger.h"
#include <string>

namespace WylesLibs::Json {

// TODO: remember to maybe support mixed type arrays? lol
//  also, zero values? Didn't really think about that... I think implementation supports it...

//  "There is no requirement that the values in an array be of the same type." - unknown source lol...
//  I think it might be implied?, so maybe check for that explicitly lol

// if we are to support mixed type arrays?
// major design flaw? --- like should parser need mapper/class information ahead of time?
//  fuck lol...
// because you want to fail fast? by information I mean accepted types... lol for each array... 
//  hm.....

//  or should mapper just multiplex.... somehow.... how do you map key to... yeah maybe if we make user keep a map

// hmmm... yeah that might be the better solution anyways... because some sort of map of that information needs to be created regardless....
//  so let's not limit to single typed arrays... and multiplex... 

// okay, so now how can I cleanly do that... this is not that...

// We might not need to fail fast because object structure is likely negotiated ahead of time?, 
    // same argument applies for not supporting mixed type arrays...

//  common ground, mapper drops types not supported by class... done....

//  yeah, that's it... support mixed type JSON arrays, drop types not supported by class... and it's up to developer to make sure we support all expected types...

//      I think i'm conflating the parsing and mapping... which I originally planned to keep as separate things but they are tightly coupled so maybe more coupling is needed?
//      for a "better" solution?

// decisions, decisions....

// multiplexing idea at class/object 
    // okay, following information gets passed to mapper function...
    //  type -> classarr
    //  type2 -> classarr2
    //  
    //  and mapper function performs the following...
    //  so element of "arr" of type -> classarr
    //     element of "arr" of type -> classarr2 

// ........................................................

// so key to multiplexing drop unsupported array types... like we do for variables not in json object... we just don't set them...

// alot to unpack here... let's sleep on this...

//  
//
//
//
//
//
//  more abstraction built on existing functions to support multiplexing...

static void setVariableFromJsonValue(JsonValue * value, bool& obj_value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == BOOLEAN) {
        obj_value = ((JsonBoolean *)value)->getValue();
        validation_count++;
    }
}

static void setVariableFromJsonValue(JsonValue * value, double& obj_value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == NUMBER) {
        obj_value = ((JsonNumber *)value)->getValue();
        validation_count++;
    }
}

static void setVariableFromJsonValue(JsonValue * value, std::string& obj_value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == STRING) {
        obj_value = ((JsonString *)value)->getValue();
        validation_count++;
    }
}

template<class T>
static T setVariableFromJsonValue(JsonValue * value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    loggerPrintf(LOGGER_DEBUG, "Setting object variable. %d\n", type);
    if (type == OBJECT) {
        validation_count++;
        return T((JsonObject *)value);
    } else {
        return T();
    }
}

// TO or FROM? English is englishing...
static void setArrayVariablesFromJsonValue(JsonValue * value, std::vector<bool>& arr, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array = (JsonArray *)value;
        // TODO: can this be abstracted out? yeah, basically, this for all boolean, number ans string arrays...
    
        //  object types, can we template?
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = (*array)[i];
            JsonType array_type = array_value->type;
            // TODO: can be object, number, double, etc... no nested arrays for now...
    
            // if (arrayJsonType == ARRAY) { // ignore ARRAY types for now...
            // } else if (arrayJsonType == OBJECT) { // ignore OBJECT also for this vector<bool> type...
            // } else {
            if (array_type == BOOLEAN) {
                // T& = arr[i]
                // because then templating issues? LMAO... reference!
                arr.push_back(false);
                // bool& element { arr[i] };
                bool element = false;
                // TODO:
                // if it turns out this is undefined by CPP... then need to get template specialization working...
                setVariableFromJsonValue(array_value, element, array_validation_count);
            }
        }
        if (array_validation_count != array->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

static void setArrayVariablesFromJsonValue(JsonValue * value, std::vector<double>& arr, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == NUMBER) {
                arr.push_back(0);
                // T& = arr[i]
                double& element { arr[i] };
                setVariableFromJsonValue(array_value, element, array_validation_count);
            }
        }
        if (array_validation_count != array->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

static void setArrayVariablesFromJsonValue(JsonValue * value, std::vector<std::string>& arr, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == STRING) {
                arr.push_back("");
                // T& = arr[i]
                std::string& element { arr[i] };
                setVariableFromJsonValue(array_value, element, array_validation_count);
            }
        }
        if (array_validation_count != array->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

// TODO: how to make sure, <bool>, <string>, etc aren't mapped to template? might be a compiler error...
template<class T>
static void setArrayVariablesFromJsonValue(JsonValue * value, std::vector<T>& arr, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        loggerPrintf(LOGGER_DEBUG, "cast?\n");
        JsonArray * array = (JsonArray *)value;
        loggerPrintf(LOGGER_DEBUG, "Before loop\n");
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == OBJECT) {
                loggerPrintf(LOGGER_DEBUG, "Adding object to array...\n");
                arr->push_back(setVariableFromJsonValue<T>(array_value, array_validation_count));
            }
        }
        if (array_validation_count != array->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

template<class T>
static void setArrayVariablesFromJsonValue(JsonValue * value, std::vector<bool> * bool_arr, 
    std::vector<double> * num_arr, std::vector<std::string> * str_arr, 
    std::vector<T> * t_arr, size_t& validation_count) {
    // TODO: how does cout print? type needs to implement some function...
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == BOOLEAN && bool_arr != nullptr) {
                bool_arr->push_back(false);
                // T& = arr[i]
                // bool& element { bool_arr->at(i) };
                setVariableFromJsonValue(array_value, bool_arr->at(i), array_validation_count);
            } else if (array_type == NUMBER && num_arr != nullptr) {
                num_arr->push_back(0);
                // T& = arr[i]
                double& element { num_arr->at(i) };
                setVariableFromJsonValue(array_value, element, array_validation_count);
            } else if (array_type == STRING && str_arr != nullptr) {
                str_arr->push_back("");
                // T& = arr[i]
                std::string& element { str_arr->at(i) };
                setVariableFromJsonValue(array_value, element, array_validation_count);
            } else if (array_type == OBJECT && t_arr != nullptr) {
                loggerPrintf(LOGGER_DEBUG, "Adding object to array...\n");
                t_arr->push_back(setVariableFromJsonValue<T>(array_value, array_validation_count));
            }
        }
        if (array_validation_count != array->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

}
#endif 