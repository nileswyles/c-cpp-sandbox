#include "http.h"

void websocketUpgrader() {}

void mainRequestProcessor(HttpRequest request) {
    // if (contentType == application/json) {
    // }

    // map
    //   resource_path -> JsonObject, request_handler(JsonObject)...
    //
    // request -> 

    // okay, so that works for application/json, but for other content?
    //  so maybe,

    //  map
    //     resource_path -> Array<uint8_t>, request_handler<Array<uint8_t>>
    //  contentType == 

    // alright so, http connection class is responsible for:
    //      calling the resource handler...
    //          question is selection...
    //              - resource path
    //              - content type
    //              or ideally a combination of both?

    //          can this also be defined by caller?
    //              definitely, yes programatically (passing "function" to connection class)
    //              definitely, yes declaratively (passing "config" to connection class)

    //              which is more work for the implementer? 
    //                  let's explore both options...
    //                      programatically:
    //                          one function that can then do whatever....
    //  
    //                      declaratively:
    //                          This requires some planning... maybe defining how it will be used...

    //                          config == representation of the following table
    //                              table:
    //                                  resource_path, content_type, resource_handler 

    //                                  resource_path, content_type, resource_handler, arg_type // (too strong lol)

    //                                  resource_path, content_type, (ResourceObjectHandler) // (too strong lol)

    //                                  ResourceObjectHandler
    //                                      type  // (too strong lol)

    //                                  ResourceObjectHandlerJson
    //                                      type  // (too strong lol)

    //                                  ... 

    //                                  ResourceObject
    //                                      resource_path
    //                                      content_type
    //                                      ResourceObjectHandler
    //                                  
    //                          so more generally, 
    //                              this is matter of "serializing" (pseudo-serializing) 
    //                              (strongly typed to semi-weakly typed (intermediate) to very-weakly typed).. parsing but not fully...

    //                          LMAO...
    //                          what is a string really?
    //                          
    //                          Okay, so let's think about performance implications of this abstraction/generalization... 
    //                              Space/time complexity is 

    //                          constant iteration is draining!!!!!
    //                          
    //                          but there's probably value in obfuscating... the mystery in the magic...?

    //                          hmm... fuck it... lol... thinking about this too much... it really depends...

    //                  building that datastructure is probably just as much work as programatically so let's not go the declarative route for this one!
    //                      can I generalize and just go straight to this for things in CPP (strongly typed, no reflection?)

    //                       - to scared to jump off that ledge... does that make me weak? lmao, the irony...
    //                  
    //                  lol... blehhhhhhhhhhhhh...... derppppppppp....

    // what a balancing act

}

int main(int argc, char * argv[]) {
    int ret = -1;
    if (argc == 3) {
        HttpConnection connection(requestProcessor);
        server_listen(argv[1], atoi(argv[2]), connection.onConnection);
        ret = 0;
    } 
    return ret;
}