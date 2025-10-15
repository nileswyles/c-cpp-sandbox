#include "parser/json/json_object.h"

#include <stdexcept>

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_JSON_OBJECT
#define LOGGER_LEVEL_JSON_OBJECT GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_JSON_OBJECT
#define LOGGER_JSON_OBJECT 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_OBJECT

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_JSON_OBJECT
#include "logger.h"

using namespace WylesLibs::Parser::Json;

JsonObject::JsonObject(std::initializer_list<JsonNode> compile, size_t depth): JsonObject(depth) {
    for (auto node: compile) {
        keys.push_back(node.key);
        if (node.boolean_value != nullptr) {
            values.addValue((JsonValue *)new JsonBoolean(*node.boolean_value));
        } else if (node.number_value != nullptr) {
            values.addValue((JsonValue *)new JsonNumber(*node.number_value));
        } else if (node.string_value != nullptr) {
            values.addValue((JsonValue *)new JsonString(*node.string_value));
        } else if (node.vector_value != nullptr) {
            JsonArray * arr = new JsonArray();
            arr->compile(*node.vector_value);
            values.addValue((JsonValue *)arr);
        } else if (node.object_value.size() > 0) {
            values.addValue((JsonValue *)new JsonObject(node.object_value, ++depth));
        } else {
            loggerPrintf(LOGGER_INFO, "Woah\n");
            std::runtime_error("Woah woah");
        }
    }
}

JsonObject::JsonObject(size_t depth): depth(depth), JsonValue(WylesLibs::Parser::Json::OBJECT) {
    if (depth > MAX_JSON_DEPTH) {
        throw std::runtime_error("JsonObject creation error... TOO MUCH DEPTH!");
    }
    values.depth = depth;
}

std::string JsonObject::toJsonString() {
    std::string s("{\n");
    for (size_t i = 0; i < this->keys.size(); i++) {
        s += Json::spacing[this->depth];
        s += "\"";
        s += this->keys[i];
        s += "\": ";

        JsonValue * value = this->values.at(i);
        JsonType type = value->type;
        if (type == BOOLEAN) {
            s += ((JsonBoolean *)value)->toJsonString();
        } else if (type == NUMBER) {
            s += ((JsonNumber *)value)->toJsonString();
        } else if (type == STRING) {
            s += ((JsonString *)value)->toJsonString();
        } else if (type == OBJECT) {
            s += ((JsonObject *)value)->toJsonString();
        } else if (type == ARRAY) {
            s += ((JsonArray *)value)->toJsonString();
        }
        if (i != this->keys.size() - 1) {
            s += ",";
        } 
        s += "\n";
    }

    s += '}\n';

    return s;
}

void JsonObject::addNode(std::string key, bool value) {
    this->addKey(key);
    this->values.addValue((JsonValue *)new JsonBoolean(value));
}

void JsonObject::addNode(std::string key, double value) {
    this->addKey(key);
    this->values.addValue((JsonValue *)new JsonNumber(value));
}

void JsonObject::addNode(std::string key, jstring value) {
    this->addKey(key);
    this->values.addValue((JsonValue *)new JsonString(value));
}

void JsonObject::addNode(std::string key, JsonArray * value) {
    this->addKey(key);
    this->values.addValue((JsonValue *)value);
}

void JsonObject::addNode(std::string key, JsonObject * value) {
    this->addKey(key);
    this->values.addValue((JsonValue *)value);
}

void JsonObject::removeNode(std::string key) {
    size_t i = this->find(key);

    size_t idx = 0;
    auto end = this->keys.end();
    for (auto it = this->keys.begin(); it != end; ++it) {
        if (idx++ == i) {
            this->keys.erase(it);
            break;
        }
    }
    if (idx < this->numValues()) {
        this->values.remove(idx);
    }
}

JsonValue * JsonObject::at(std::string key) {
    size_t i = this->find(key);
    return i == SIZE_MAX ? throw std::runtime_error("Key not found.") : this->values.at(i);
}

void JsonObject::addKey(std::string key) {
    this->keys.push_back(key);
    loggerPrintf(LOGGER_DEBUG, "Added json key! @ %s\n", key.c_str());
}