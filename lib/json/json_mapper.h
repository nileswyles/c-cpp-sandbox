#ifndef WYLESLIBS_JSON_MAPPER_H
#define WYLESLIBS_JSON_MAPPER_H

#include "logger.h"
#include "json_parser.h"
#include <string>

namespace WylesLibs::Json {

// LOL... cpp*?

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
                bool element = arr[i]; // hmmm.....
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
                // T& = arr[i]
                double element = arr[i]; // hmmm.....
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
                // T& = arr[i] - 
                std::string element = arr[i]; // hmmm.....
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
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == OBJECT) {
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

}
#endif 