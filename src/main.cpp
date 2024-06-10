#include <iostream>
#include <sys/socket.h> // socket, bind, accept, setsockopt
#include <netinet/in.h> // sockaddr_in
#include <strings.h> // bzero, memset
#include <arpa/inet.h> // inet_pton
#include <cstring>

#include "config.h"

int main(int argc, char **argv) {
    Config c = Config::init(argc, argv);
    int listen_port = c.get_listen_port();

    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        std::cout << "error when create socket" << std::endl;
        exit(1);
    }

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(listen_port);
    inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);

    int result = bind(server_socket_fd, (struct sockaddr*)&address, sizeof(address));
    if (result < 0) {
        std::cout << "error when bind" << std::endl;
        exit(1);
    }

    int reuse = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    result = listen(server_socket_fd, 100);
    if (result < 0) {
        std::cout << "error when listen" << std::endl;
        exit(1);
    }

    struct sockaddr_in accept_addr;
    socklen_t accept_len = sizeof(accept_addr);
    while (true) {
        int accept_fd = accept(server_socket_fd, (struct sockaddr*)&accept_addr, &accept_len);
        if (accept_fd < 0) {
            std::cout << "error when accept" << std::endl;
            exit(1);
        }

        char buffer[100];
        for (;;) {
            memset(buffer, '\0', 100);
            result = recv(accept_fd, buffer, 99, 0);
            if (result <= 0) exit(result);
            std::cout << buffer << std::flush;
        }
    }
}
