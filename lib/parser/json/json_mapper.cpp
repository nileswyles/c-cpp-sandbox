#include "json_mapper.h"

using namespace WylesLibs::Parser::Json;

// template<class T>
// T WylesLibs::Parser::Json::setVariableFromJsonValue(JsonValue * value) {
//     JsonType type = value->type;
//     loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
//     if (type == OBJECT) {
//         return T((JsonObject *)value);
//     } else {
//         loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
//         throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
//     }
// }

template<>
bool WylesLibs::Parser::Json::setVariableFromJsonValue<bool>(JsonValue * value) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == BOOLEAN) {
        bool s = ((JsonBoolean *)value)->getValue();
        loggerPrintf(LOGGER_DEBUG, "value: %d\n", s);
        return s;
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
double WylesLibs::Parser::Json::setVariableFromJsonValue<double>(JsonValue * value) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == NUMBER) {
        double s = ((JsonNumber *)value)->getValue();
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", s);
        return s;
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
    return 0;
}

template<>
std::string WylesLibs::Parser::Json::setVariableFromJsonValue<std::string>(JsonValue * value) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == STRING) {
        std::string s = ((JsonString *)value)->getValue();
        loggerPrintf(LOGGER_DEBUG, "value: %s\n", s.c_str());
        return s;
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<class T>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue(JsonValue * value, std::vector<T>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == OBJECT) {
                arr->push_back(setVariableFromJsonValue<T>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<bool>(JsonValue * value, std::vector<bool>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = (*array)[i];
            JsonType array_type = array_value->type;
            if (array_type == BOOLEAN) {
                arr.push_back(setVariableFromJsonValue<bool>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<double>(JsonValue * value, std::vector<double>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == NUMBER) {
                arr.push_back(setVariableFromJsonValue<double>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<std::string>(JsonValue * value, std::vector<std::string>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == STRING) {
                arr.push_back(setVariableFromJsonValue<std::string>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}