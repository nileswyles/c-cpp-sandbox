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


/// this is something... aint it...

// hmm... so how would one implement this?
template<class T>
T setVariableFromJsonValue(JsonValue * value, size_t& validation_count);

template<>
bool setVariableFromJsonValue<bool>(JsonValue * value, size_t& validation_count);

template<>
double setVariableFromJsonValue<double>(JsonValue * value, size_t& validation_count);

template<>
std::string setVariableFromJsonValue<std::string>(JsonValue * value, size_t& validation_count);

template<class T>
void setArrayVariablesFromJsonValue(JsonValue * value, std::vector<T>& arr, size_t& validation_count);

template<>
void setArrayVariablesFromJsonValue<bool>(JsonValue * value, std::vector<bool>& arr, size_t& validation_count);

template<>
void setArrayVariablesFromJsonValue<double>(JsonValue * value, std::vector<double>& arr, size_t& validation_count);

template<>
void setArrayVariablesFromJsonValue<std::string>(JsonValue * value, std::vector<std::string>& arr, size_t& validation_count);

}
#endif 