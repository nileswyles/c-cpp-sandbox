#ifndef WYLESLIBS_JSON_MAPPER_H
#define WYLESLIBS_JSON_MAPPER_H

#include "json_parser.h"
#include "json_object.h"
#include "json_array.h"
#include "jstring.h"
#include "array.h"

#include <string>

#ifndef LOGGER_JSON_MAPPER
#define LOGGER_JSON_MAPPER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_MAPPER
#include "logger.h"

namespace WylesLibs::Parser::Json {
    // verdict on supporting multiple types in json arrrays...
    //  the parser will not restrict to single types... that said, I don't plan on providing an abstraction for the multiplexing.
    //  it's up to the developer to process the parsed intermediate on a case-by-case basis. 
    //      - redacted wrt null-type support, provided an easy way to map those assuming pointers to objects are your thing. - 

    // template<class T>
    // T setVariableFromJsonValue(JsonValue * value);

    // lolllllll? 
    static const std::string ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE("Failed to set variable from json value. Invalid type.");

    template<class T>
    T setVariableFromJsonValue(JsonValue * value) {
        JsonType type = value->type;
        loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
        if (type == OBJECT) {
            return T(ESharedPtr<JsonObject>(dynamic_cast<JsonObject *>(value)));
        } else {
            loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
            throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
        }
    }

    template<class T>
    T * setPointerVariableFromJsonValue(JsonValue * value) {
        JsonType type = value->type;
        loggerPrintf(LOGGER_DEBUG, "value type: %d\n", type);
        if (type == OBJECT) {
            return new T(ESharedPtr<JsonObject>(dynamic_cast<JsonObject *>(value)));
        } else if (type == NULL_TYPE) {
            return nullptr;
        } else {
            loggerPrintf(LOGGER_INFO, "%s\n", ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE.c_str());
            throw std::runtime_error(ERR_MSG_SET_VARIABLE_FROM_JSON_VALUE);
        }
    }

    template<>
    bool setVariableFromJsonValue<bool>(JsonValue * value);

    template<>
    double setVariableFromJsonValue<double>(JsonValue * value);

    template<>
    jstring setVariableFromJsonValue<jstring>(JsonValue * value);

    template<class T>
    void setArrayVariablesFromJsonValue(JsonValue * value, SharedArray<T>& arr);

    template<class T>
    void setArrayPointerVariablesFromJsonValue(JsonValue * value, SharedArray<T *>& arr);

    template<>
    void setArrayVariablesFromJsonValue<bool>(JsonValue * value, SharedArray<bool>& arr);

    template<>
    void setArrayVariablesFromJsonValue<double>(JsonValue * value, SharedArray<double>& arr);

    template<>
    void setArrayVariablesFromJsonValue<jstring>(JsonValue * value, SharedArray<jstring>& arr);

    // For when there's alignment... see retroarer repo.
    template<class T>
    void setMapVariableFromJsonValue(JsonValue * value, std::string key, std::map<std::string, T>& map);

    template<class T>
    void setMapPointerVariableFromJsonValue(JsonValue * value, std::string key, std::map<std::string, T *>& map);

    template<>
    void setMapVariableFromJsonValue<bool>(JsonValue * value, std::string key, std::map<std::string, bool>& map);

    template<>
    void setMapVariableFromJsonValue<double>(JsonValue * value, std::string key, std::map<std::string, double>& map);

    template<>
    void setMapVariableFromJsonValue<jstring>(JsonValue * value, std::string key, std::map<std::string, jstring>& map);
}
#endif 