#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/ip.h>
#include <unistd.h>


int main(int argc, char * argv[]) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        // check errno for error
        printf("error creating socket\n");
    } else {
        // REMEMBER, Big-Endianness is prominent in network protocols like IP... (network order) - send most significant byte first...
        // so, call ntohs to convert port... addr is already defined in network order.
        static const uint8_t addr[] = {127, 0, 0, 1}; // addr uint32
        struct sockaddr_in address = {
            AF_INET,
            ntohs(8080),
            *(in_addr_t*)addr,
        };

        if (bind(fd, (struct sockaddr *)(&address), sizeof(struct sockaddr_in)) == -1) {
            // check errno for error
            printf("error binding, %d\n", errno);
        } else {
            if (listen(fd, 1024) == -1) {
                printf("Error listening\n");
            } else {
                int conn = accept(fd, NULL, NULL);
                if (conn == -1) {
                    printf("ERROR ACCEPTING connections\n");
                } else {
                    printf("conn!\n");
                    sleep(30);
                }
            }
        }
        
    }
    return 0;
}