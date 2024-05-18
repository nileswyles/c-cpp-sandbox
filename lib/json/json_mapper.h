#ifndef WYLESLIBS_JSON_MAPPER_H
#define WYLESLIBS_JSON_MAPPER_H

#include "parser/json_parser.h"
#include "parser/json_object.h"
#include "parser/json_array.h"

#include <string>

#ifndef LOGGER_JSON_MAPPER
#define LOGGER_JSON_MAPPER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_MAPPER
#include "logger.h"

namespace WylesLibs::Json {

// verdict on supporting multiple types in json arrrays...
//  the parser will not restrict to single types... that said, I don't plan on providing an abstraction for the multiplexing.
//  it's up to the developer to process the parsed intermediate on a case-by-case basis.  

template<class T>
T setVariableFromJsonValue(JsonValue * value, size_t& validation_count) {
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

template<>
bool setVariableFromJsonValue<bool>(JsonValue * value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == BOOLEAN) {
        validation_count++;
        return ((JsonBoolean *)value)->getValue();
    }
    return false;
}

template<>
double setVariableFromJsonValue<double>(JsonValue * value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == NUMBER) {
        validation_count++;
        return ((JsonNumber *)value)->getValue();
    }
    return 0;
}

template<>
std::string setVariableFromJsonValue<std::string>(JsonValue * value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == STRING) {
        validation_count++;
        return ((JsonString *)value)->getValue();
    }
    return "";
}

template<class T>
void setArrayVariablesFromJsonValue(JsonValue * value, std::vector<T>& arr, size_t& validation_count) {
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

template<>
void setArrayVariablesFromJsonValue<bool>(JsonValue * value, std::vector<bool>& arr, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = (*array)[i];
            JsonType array_type = array_value->type;
            if (array_type == BOOLEAN) {
                arr.push_back(setVariableFromJsonValue<bool>(array_value, array_validation_count));
            }
        }
        if (array_validation_count != array->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

template<>
void setArrayVariablesFromJsonValue<double>(JsonValue * value, std::vector<double>& arr, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == NUMBER) {
                arr.push_back(setVariableFromJsonValue<double>(array_value, array_validation_count));
            }
        }
        if (array_validation_count != array->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

template<>
void setArrayVariablesFromJsonValue<std::string>(JsonValue * value, std::vector<std::string>& arr, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == STRING) {
                arr.push_back(setVariableFromJsonValue<std::string>(array_value, array_validation_count));
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