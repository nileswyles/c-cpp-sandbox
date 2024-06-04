#include "json_mapper.h"

using namespace WylesLibs::Parser::Json;

// template<class T>
// T WylesLibs::Parser::Json::setVariableFromJsonValue(JsonValue * value) {
//     JsonType type = value->type;
//     loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
//     if (type == OBJECT) {
//         return T((JsonObject *)value);
//     } else {
//         loggerPrintf(LOGGER_ERROR, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
//         throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
//     }
// }

template<>
bool WylesLibs::Parser::Json::setVariableFromJsonValue<bool>(JsonValue * value) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == BOOLEAN) {
        return ((JsonBoolean *)value)->getValue();
    } else {
        loggerPrintf(LOGGER_ERROR, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
double WylesLibs::Parser::Json::setVariableFromJsonValue<double>(JsonValue * value) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == NUMBER) {
        return ((JsonNumber *)value)->getValue();
    } else {
        loggerPrintf(LOGGER_ERROR, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
    return 0;
}

template<>
std::string WylesLibs::Parser::Json::setVariableFromJsonValue<std::string>(JsonValue * value) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == STRING) {
        return ((JsonString *)value)->getValue();
    } else {
        loggerPrintf(LOGGER_ERROR, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
    return "";
}

template<class T>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue(JsonValue * value, std::vector<T>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == OBJECT) {
                arr->push_back(setVariableFromJsonValue<T>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_ERROR, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<bool>(JsonValue * value, std::vector<bool>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = (*array)[i];
            JsonType array_type = array_value->type;
            if (array_type == BOOLEAN) {
                arr.push_back(setVariableFromJsonValue<bool>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_ERROR, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<double>(JsonValue * value, std::vector<double>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == NUMBER) {
                arr.push_back(setVariableFromJsonValue<double>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_ERROR, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<std::string>(JsonValue * value, std::vector<std::string>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (int i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == STRING) {
                arr.push_back(setVariableFromJsonValue<std::string>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_ERROR, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}