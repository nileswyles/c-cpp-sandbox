#include "json_array.h"
#include "json_object.h"
#include "json_mix.h"

using namespace WylesLibs::Parser::Json;

void JsonArray::compile(std::vector<JsonNode> arr) {
    for (auto el: arr) {
        if (el.boolean_value != nullptr) {
            this->addValue((JsonValue *)new JsonBoolean(*el.boolean_value));
        } else if (el.number_value != nullptr) {
            this->addValue((JsonValue *)new JsonNumber(*el.number_value));
        } else if (el.string_value != nullptr) {
            this->addValue((JsonValue *)new JsonString(*el.string_value));
        } else if (el.vector_value != nullptr) {
            JsonArray * arr = new JsonArray();
            arr->compile(*el.vector_value);
            this->addValue((JsonValue *)arr);
        } else if (el.object_value.size() > 0) {
            this->addValue((JsonValue *)new JsonObject(el.object_value, ++depth));
        } else {
            loggerPrintf(LOGGER_INFO, "Woah\n");
            std::runtime_error("Woah woah");
        }
    }
}

void JsonArray::remove(size_t i) {
    size_t idx = 0;
    auto end = this->end();
    for (auto it = this->begin(); it != end; ++it) {
        if (idx++ == i) {
            this->erase(it);
            break;
        }
    }
}

std::string JsonArray::toJsonString() {
    std::string s("[\n");
    for (size_t i = 0; i < this->size(); i++) {
        JsonValue * value = this->at(i);
        JsonType type = value->type;
        s += Json::spacing[this->depth];
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
        if (i != this->size() - 1) {
            s += ",";
        } 
        s += "\n";
    }
    s += ']\n';
    return s;
}
