#include "json_mapper.h"

using namespace WylesLibs::Json;

extern void setVariableFromJsonValue(JsonType type, JsonValue * json_value, bool& obj_value, size_t& validation_count) {
    if (type == BOOLEAN) {
        obj_value = ((JsonBoolean *)json_value)->getValue();
        validation_count++;
    }
}

extern void setVariableFromJsonValue(JsonType type, JsonValue * json_value, double& obj_value, size_t& validation_count) {
    if (type == NUMBER) {
        obj_value = ((JsonNumber *)json_value)->getValue();
        validation_count++;
    }
}

extern void setVariableFromJsonValue(JsonType type, JsonValue * json_value, std::string& obj_value, size_t& validation_count) {
    if (type == STRING) {
        obj_value = ((JsonString *)json_value)->getValue();
        validation_count++;
    }
}

// TO or FROM? English is englishing...
extern void setArrayVariablesFromJsonValue(JsonType type, JsonValue * json_value, std::vector<bool>& arr, size_t& validation_count) {
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array_value = (JsonArray *)json_value;
        // TODO: can this be abstracted out? yeah, basically, this for all boolean, number ans string arrays...
    
        //  object types, can we template?
        for (int i = 0; i < array_value->size(); i++) {
            JsonValue * array_json_value = (*array_value)[i];
            JsonType array_json_type = array_json_value->type;
            // TODO: can be object, number, double, etc... no nested arrays for now...
    
            // if (arrayJsonType == ARRAY) { // ignore ARRAY types for now...
            // } else if (arrayJsonType == OBJECT) { // ignore OBJECT also for this vector<bool> type...
            // } else {
            if (array_json_type == BOOLEAN) {
                // T& = arr[i]
                bool element = arr[i]; // hmmm.....
                setVariableFromJsonValue(array_json_type, array_json_value, element, array_validation_count);
            }
        }
        if (array_validation_count != array_value->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

extern void setArrayVariablesFromJsonValue(JsonType type, JsonValue * json_value, std::vector<double>& arr, size_t& validation_count) {
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array_value = (JsonArray *)json_value;
        for (int i = 0; i < array_value->size(); i++) {
            JsonValue * array_json_value = array_value->at(i);
            JsonType array_json_type = array_json_value->type;
            if (array_json_type == NUMBER) {
                // T& = arr[i]
                double element = arr[i]; // hmmm.....
                setVariableFromJsonValue(array_json_type, array_json_value, element, array_validation_count);
            }
        }
        if (array_validation_count != array_value->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

extern void setArrayVariablesFromJsonValue(JsonType type, JsonValue * json_value, std::vector<std::string>& arr, size_t& validation_count) {
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array_value = (JsonArray *)json_value;
        for (int i = 0; i < array_value->size(); i++) {
            JsonValue * array_json_value = array_value->at(i);
            JsonType array_json_type = array_json_value->type;
            if (array_json_type == STRING) {
                // T& = arr[i] - 
                std::string element = arr[i]; // hmmm.....
                setVariableFromJsonValue(array_json_type, array_json_value, element, array_validation_count);
            }
        }
        if (array_validation_count != array_value->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}
