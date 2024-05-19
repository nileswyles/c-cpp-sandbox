#include "json_mapper.h"

using namespace WylesLibs::Json;

template<class T>
T WylesLibs::Json::setVariableFromJsonValue(JsonValue * value, size_t& validation_count) {
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
bool WylesLibs::Json::setVariableFromJsonValue<bool>(JsonValue * value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == BOOLEAN) {
        validation_count++;
        return ((JsonBoolean *)value)->getValue();
    }
    return false;
}

template<>
double WylesLibs::Json::setVariableFromJsonValue<double>(JsonValue * value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == NUMBER) {
        validation_count++;
        return ((JsonNumber *)value)->getValue();
    }
    return 0;
}

template<>
std::string WylesLibs::Json::setVariableFromJsonValue<std::string>(JsonValue * value, size_t& validation_count) {
    Json::JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == STRING) {
        validation_count++;
        return ((JsonString *)value)->getValue();
    }
    return "";
}

template<class T>
void WylesLibs::Json::setArrayVariablesFromJsonValue(JsonValue * value, std::vector<T>& arr, size_t& validation_count) {
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
void WylesLibs::Json::setArrayVariablesFromJsonValue<bool>(JsonValue * value, std::vector<bool>& arr, size_t& validation_count) {
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
void WylesLibs::Json::setArrayVariablesFromJsonValue<double>(JsonValue * value, std::vector<double>& arr, size_t& validation_count) {
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
void WylesLibs::Json::setArrayVariablesFromJsonValue<std::string>(JsonValue * value, std::vector<std::string>& arr, size_t& validation_count) {
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