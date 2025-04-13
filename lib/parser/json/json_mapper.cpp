#include "parser/json/json_mapper.h"

using namespace WylesLibs::Parser::Json;

// TODO: need to figure out this non-sense.

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
jstring WylesLibs::Parser::Json::setVariableFromJsonValue<jstring>(JsonValue * value) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == STRING) {
        jstring s = ((JsonString *)value)->getValue();
        loggerPrintf(LOGGER_DEBUG, "value: %s\n", s.c_str());
        return s;
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<class T>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue(JsonValue * value, SharedArray<T>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == OBJECT) {
                arr.append(setVariableFromJsonValue<T>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<class T>
void WylesLibs::Parser::Json::setArrayPointerVariablesFromJsonValue(JsonValue * value, SharedArray<T *>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == OBJECT) {
                arr.append(setPointerVariableFromJsonValue<T>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<bool>(JsonValue * value, SharedArray<bool>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = (*array)[i];
            JsonType array_type = array_value->type;
            if (array_type == BOOLEAN) {
                arr.append(setVariableFromJsonValue<bool>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<double>(JsonValue * value, SharedArray<double>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == NUMBER) {
                arr.append(setVariableFromJsonValue<double>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setArrayVariablesFromJsonValue<jstring>(JsonValue * value, SharedArray<jstring>& arr) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == ARRAY) {
        JsonArray * array = (JsonArray *)value;
        for (size_t i = 0; i < array->size(); i++) {
            JsonValue * array_value = array->at(i);
            JsonType array_type = array_value->type;
            if (array_type == STRING) {
                arr.append(setVariableFromJsonValue<jstring>(array_value));
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<class T>
void WylesLibs::Parser::Json::setMapVariableFromJsonValue(JsonValue * value, std::string key, std::map<std::string, T>& map) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == OBJECT) {
        JsonObject * obj = (JsonObject *)value;
        for (auto node: *obj) {
            JsonValue * el_value = node.second;
            if (el_value->type == OBJECT) {
                map[key] = setVariableFromJsonValue<T>(el_value);
            } else {
                loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
                throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<class T>
void WylesLibs::Parser::Json::setMapPointerVariableFromJsonValue(JsonValue * value, std::string key, std::map<std::string, T *>& map) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == OBJECT) {
        JsonObject * obj = (JsonObject *)value;
        for (auto node: *obj) {
            JsonValue * el_value = node.second;
            if (el_value->type == OBJECT) {
                map[key] = setVariableFromJsonValue<T>(el_value);
            } else {
                loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
                throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setMapVariableFromJsonValue<bool>(JsonValue * value, std::string key, std::map<std::string, bool>& map) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == Parser::Json::OBJECT) {
        JsonObject * obj = (JsonObject *)value;
        for (auto node: *obj) {
            JsonValue * el_value = node.second;
            if (el_value->type == Parser::Json::BOOLEAN) {
                map[key] = setVariableFromJsonValue<bool>(el_value);
            } else {
                loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
                throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setMapVariableFromJsonValue<double>(JsonValue * value, std::string key, std::map<std::string, double>& map) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == OBJECT) {
        JsonObject * obj = (JsonObject *)value;
        for (auto node: *obj) {
            JsonValue * el_value = node.second;
            if (el_value->type == NUMBER) {
                map[key] = setVariableFromJsonValue<double>(el_value);
            } else {
                loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
                throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}

template<>
void WylesLibs::Parser::Json::setMapVariableFromJsonValue<jstring>(JsonValue * value, std::string key, std::map<std::string, jstring>& map) {
    JsonType type = value->type;
    loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
    if (type == OBJECT) {
        JsonObject * obj = (JsonObject *)value;
        for (auto node: *obj) {
            JsonValue * el_value = node.second;
            if (el_value->type == STRING) {
                map[key] = setVariableFromJsonValue<jstring>(el_value);
            } else {
                loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
                throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
            }
        }
    } else {
        loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
        throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
    }
}