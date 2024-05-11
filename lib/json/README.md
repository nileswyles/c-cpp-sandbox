https://www.json.org/json-en.html
CPP DataTypes:
    extends/implements... same thing? sometimes...

    extends - isa
    x> - hasa

    abstract type JSONValue
    JSONArray extends JSONValue
        x> array<JSONValue>
    JSONNumber extends JSONValue
        x> double
    JSONString extends JSONValue
        x> std::string
    JSONObject extends JSONValue
        x> array<string> keys
        x> array<JSONValue> value

class Example {
    Example(JSONObject root) {
        // then iterate over keys/values... 
        // this is specific to class right?
    }
}

so, array is basically Object without keys? Is it worth making distinction? or rather, is it worth combining types? LMAO, perspective.

Depends on where you're at, I guess...

like is JSONObject better defined as...
    JSONObject extends JSONValue
        x> array<string> keys
        x> JSONArray
or actually,
    JSONObject extends JSONArray
        x> array<string> keys

I guess, yeah that too LMAO

okay, we have the datatypes... let's parse the serialized data and write to appropriate data structures.

while(char) {
    if "{" -- new JSONObject
        if (") - start of key, new string
            if (") end of key
        if ":" = start of value
            if (") - start of string
                if (") end of string
            else if ([0-9]) -- start of number
            else if ([) -- start of array
            else if ({)
        if "}" -- end of JSONObject
}

so, can be split up as follows...
    parseJSONObject
    parseJSONNumber
    parseJSONArray
    parseJSONString

so, basically main loop finds start of these, detects when to enter these functions (dynamically scales) and enters subfunction as needed to parse that type... and we're done?

that was easy...