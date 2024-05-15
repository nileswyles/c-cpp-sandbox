#ifndef WYLESLIBS_JSON_H
#define WYLESLIBS_JSON_H

#include "logger.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace WylesLibs::Json {

typedef enum JsonType {
    NULL_TYPE = 0,
    BOOLEAN,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT,
} JsonType;

class JsonBase {
    public:
        // JsonClass() {};
        // JsonClass(JsonObject * obj) {};
        // virtual ~JsonClass() {};

        // virtual JsonClass() {};
        // virtual JsonClass(JsonObject * obj) {};

        // we want a compiler error, if this function isn't implemented in derived classes..
        virtual std::string toJsonString();
};

class JsonValue {
    public:
        JsonType type;
        JsonValue(): type(NULL_TYPE) {}
        JsonValue(JsonType derived_type): type(derived_type) {}
        // TODO: confirm...
        // virtual ~JsonValue() {}
};

typedef void(ProcessObjectFunc)(std::string key, JsonValue * value);
typedef void(ProcessValueFunc)(JsonValue * value);

class JsonBoolean: public JsonValue {
    private:
        bool boolean;
    public:
        JsonBoolean(bool boolean): boolean(boolean), JsonValue(BOOLEAN) {}

        bool getValue() {
            return this->boolean;
        }
};

class JsonNumber: public JsonValue {
    private:
        double number;
    public:
        JsonNumber(double number): number(number), JsonValue(NUMBER) {}

        double getValue() {
            return this->number;
        }
};

class JsonString: public JsonValue {
    private:
        std::string s;
    public:
        JsonString(std::string s): s(s), JsonValue(STRING) {}
        
        std::string getValue() {
            return this->s;
        }
};

class JsonArray: public JsonValue, public std::vector<JsonValue *> {
    public:
        JsonArray(): JsonValue(ARRAY), std::vector<JsonValue *>() {}
        ~JsonArray() {
            for (size_t i = 0; i < this->size(); i++) {
                loggerPrintf(LOGGER_DEBUG, "Making sure to free pointer! @ %p\n", this->at(i));
                // because this->[] isn't valid :)
                delete this->at(i);
            }
        }

        void addValue(JsonValue * value) {
            this->push_back(value);
            loggerPrintf(LOGGER_DEBUG, "Added json value object! @ %p\n", value);
        }
};

class JsonObject: public JsonValue {
    public:
        // TODO: hide this? make interface better...
        std::vector<std::string> keys; 
        JsonArray values;
        JsonObject(): JsonValue(OBJECT) {}

        void addKey(std::string key) {
            this->keys.push_back(key);
        }
};

extern JsonValue * parse(std::string * json);

static void setVariableFromJsonValue(JsonType type, JsonValue * json_value, bool& obj_value, size_t& validation_count) {
    if (type == BOOLEAN) {
        obj_value = ((JsonBoolean *)json_value)->getValue();
        validation_count++;
    }
}

static void setVariableFromJsonValue(JsonType type, JsonValue * json_value, double& obj_value, size_t& validation_count) {
    if (type == NUMBER) {
        obj_value = ((JsonNumber *)json_value)->getValue();
        validation_count++;
    }
}

static void setVariableFromJsonValue(JsonType type, JsonValue * json_value, std::string& obj_value, size_t& validation_count) {
    if (type == STRING) {
        obj_value = ((JsonString *)json_value)->getValue();
        validation_count++;
    }
}

template<class T>
static T setVariableFromJsonValue(JsonType type, JsonValue * json_value, size_t& validation_count) {
    if (type == OBJECT) {
        validation_count++;
        return T((JsonObject *)json_value);
    } else {
        return T();
    }
}

// TO or FROM? English is englishing...
static void setArrayVariablesFromJsonValue(JsonType type, JsonValue * json_value, std::vector<bool>& arr, size_t& validation_count) {
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

static void setArrayVariablesFromJsonValue(JsonType type, JsonValue * json_value, std::vector<double>& arr, size_t& validation_count) {
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

static void setArrayVariablesFromJsonValue(JsonType type, JsonValue * json_value, std::vector<std::string>& arr, size_t& validation_count) {
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

// TODO: how to make sure, <bool>, <string>, etc aren't mapped to template? might be a compiler error...
template<class T>
static void setArrayVariablesFromJsonValue(JsonType type, JsonValue * json_value, std::vector<T>& arr, size_t& validation_count) {
    if (type == ARRAY) {
        size_t array_validation_count = 0;
        JsonArray * array_value = (JsonArray *)json_value;
        for (int i = 0; i < array_value->size(); i++) {
            JsonValue * array_json_value = array_value->at(i);
            JsonType array_json_type = array_json_value->type;
            if (array_json_type == OBJECT) {
                arr->push_back(setVariableFromJsonValue<T>(array_json_type, array_json_value, array_validation_count));
            }
        }
        if (array_validation_count != array_value->size()) {
            // ensures elements in array are correct type.
            throw std::runtime_error("Failed to create User from json object.");
        }
        validation_count++;
    }
}

// TODO: array of arbitrary objects

}
#endif