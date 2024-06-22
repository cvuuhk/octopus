#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // socket, bind, accept, setsockopt
#include <arpa/inet.h>
#include <string>

#include "event.hpp"
#include "logger.hpp"

AcceptEvent::AcceptEvent(int socket_fd) {
    this->fd = socket_fd;
}

void AcceptEvent::handleEvent() {

    struct sockaddr_in accept_addr;
    socklen_t accept_len = sizeof(accept_addr);

    int accept_fd = accept(this->fd, (struct sockaddr*)&accept_addr, &accept_len);
    if (accept_fd < 0) {
        logger.error("error when accept");
        return;
    }

    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &accept_addr.sin_addr, str, INET_ADDRSTRLEN);
    int port = ntohs(accept_addr.sin_port);

    logger.info("connected with " + std::string(str) + ":" + std::to_string(port));
}
