#include "http.h"

void mainRequestProcessor(HttpRequest request) {

    // on websocket upgrade this will not ret

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