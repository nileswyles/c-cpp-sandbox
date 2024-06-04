#ifndef WYLESLIBS_JSON_MAPPER_H
#define WYLESLIBS_JSON_MAPPER_H

#include "parser/json/json_parser.h"
#include "parser/json/json_object.h"
#include "parser/json/json_array.h"

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

template<class T>
T setVariableFromJsonValue(JsonValue * value);

template<>
bool setVariableFromJsonValue<bool>(JsonValue * value);

template<>
double setVariableFromJsonValue<double>(JsonValue * value);

template<>
std::string setVariableFromJsonValue<std::string>(JsonValue * value);

template<class T>
void setArrayVariablesFromJsonValue(JsonValue * value, std::vector<T>& arr);

template<>
void setArrayVariablesFromJsonValue<bool>(JsonValue * value, std::vector<bool>& arr);

template<>
void setArrayVariablesFromJsonValue<double>(JsonValue * value, std::vector<double>& arr);

template<>
void setArrayVariablesFromJsonValue<std::string>(JsonValue * value, std::vector<std::string>& arr);

}
#endif 