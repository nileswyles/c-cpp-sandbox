#include "json_array.h"
#include "json_object.h"

using namespace WylesLibs::Json;

std::string JsonArray::toJsonString() {
    std::string s("[");
    for (size_t i = 0; i < this->size(); i++) {
        JsonValue * value = this->at(i);
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
        if (i != this->size() - 1) {
            s += ", ";
        } 
    }
    s += ']';
    return s;
}
