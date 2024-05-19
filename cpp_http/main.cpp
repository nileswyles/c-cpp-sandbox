#include "http.h"
#include "upgrader.h"
#include "config.h"

using namespace WylesLibs;
using namespace WylesLibs::Http;

class WebsocketConnectionUpgrader: public ConnectionUpgrader {
    public:
        WebsocketConnectionUpgrader(std::string path, std::string protocol): ConnectionUpgrader(path, protocol) {}

        // this makes more sense extension of some Connection class?
        uint8_t onConnection(int conn_fd) {
            printf("Established websocket connection...\n");

            return 1;
        }
};

// HttpResponse * exampleResourceHandler(HttpRequest * request) {
//     return;
// }

// HttpResponse * example2ResourceHandler(HttpRequest * request) {
//     JsonBasedClass obj(request->content);

//     obj.blah
//     obj.blah

//     return;
// }

HttpResponse * requestProcessor(HttpRequest * request) {
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

    // insert middleware here....
    
    // need parsers for each of these at least
    //    application/x-www-form-urlencoded
    //    multipart/byteranges
    //    multipart/formdata
    //    application/json
    // if ("/example" == request.url.path && "application/json" == request.fields["content-type"]) {
    //     printf("calling resource handler for /example path\n");
    //     // call to resource handler...
    // } else if ("/example2" == request.url.path && "multipart/byteranges" == request.fields["content-type"]) {
    //     // call to resource handler...
    // } else if ("/example3" == request.url.path) {
    //     // call to resource handler...
    // }
    return nullptr;
}

static HttpConnection connection;

static uint8_t usethis(int conn_fd) {
    return connection.onConnection(conn_fd);
}

int main(int argc, char * argv[]) {
    int ret = -1;
        printf("?????????\n");
    if (argc == 3) {
        printf("???\n");
        HttpServerConfig config("./http-config.json");

        printf("???\n");
        Array<ConnectionUpgrader *> upgraders;
        WebsocketConnectionUpgrader upgrader("/testpath", "jsonrpc");
        ConnectionUpgrader * lame = &upgrader;
        printf("???\n");
        upgraders.append(lame);

        connection = HttpConnection(config, requestProcessor, upgraders); 
        printf("???\n");
        // womp womp womp... :(
        server_listen(argv[1], atoi(argv[2]), usethis);
        ret = 0;
    } 
    return ret;
}