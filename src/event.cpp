#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // socket, bind, accept, setsockopt
#include <arpa/inet.h>
#include <string>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>

#include "event.hpp"
#include "logger.hpp"
#include "server.hpp"

AcceptEvent::AcceptEvent(int fd, Server* server) {
    this->fd = fd;
    this->mask = EPOLLIN;
    this->epoll_event = new struct epoll_event;
    this->epoll_event->events = EPOLLIN;
    this->epoll_event->data.fd = fd;
    this->server = server;
}

void AcceptEvent::handleEvent() {

    struct sockaddr_in accept_addr;
    socklen_t accept_len = sizeof(accept_addr);

    int accept_fd = accept(this->fd, (struct sockaddr*)&accept_addr, &accept_len);
    if (accept_fd < 0) {
        logger.error("error when accept");
        return;
    }

    int flags;
    if ((flags = fcntl(accept_fd, F_GETFL)) == -1) {
        logger.error("error when fcntl(" + std::to_string(accept_fd) + ", F_GETFL)");
    }
    if (fcntl(accept_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        logger.error("error when set O_NONBLOCK " + std::to_string(accept_fd));
    }

    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &accept_addr.sin_addr, str, INET_ADDRSTRLEN);
    int port = ntohs(accept_addr.sin_port);

    logger.info("connected with " + std::string(str) + ":" + std::to_string(port));
    this->server->addEvent(new ReadEvent(accept_fd));
}

int AcceptEvent::getFd() {
    return this->fd;
}

int AcceptEvent::getMask() {
    return this->mask;
}

void AcceptEvent::setMask(int mask) {
    this->mask = mask;
    if(this->epoll_event != nullptr) {
        this->epoll_event->events = mask;
    }
}

struct epoll_event* AcceptEvent::getEpollEvent() {
    return this->epoll_event;
}


ReadEvent::ReadEvent(int fd) {
    this->fd = fd;
    this->mask = EPOLLIN;
    this->epoll_event = new struct epoll_event;
    this->epoll_event->events = EPOLLIN;
    this->epoll_event->data.fd = fd;
    this->server = server;
}

void ReadEvent::handleEvent() {
    struct sockaddr_in accept_addr;
    socklen_t accept_len = sizeof(accept_addr);
    while (true) {
        int result;
        char buffer[100];
        for (;;) {
            memset(buffer, '\0', 100);
            result = recv(this->fd, buffer, 99, 0);
            if (result == -1) {
                if (errno == EAGAIN) break;
                // logger.error("error when recv: " + std::string(strerror(errno)));
                exit(result);
            } else if (result == 0) {
                // logger.info("client disconnected")
                // this->server->removeEvent(this);
                close(this->fd);
                break;
            } else {
                std::cout << buffer << std::flush;
            }
        }
    }
}

int ReadEvent::getFd() {
    return this->fd;
}

int ReadEvent::getMask() {
    return this->mask;
}

void ReadEvent::setMask(int mask) {
    this->mask = mask;
    if(this->epoll_event != nullptr) {
        this->epoll_event->events = mask;
    }
}

struct epoll_event* ReadEvent::getEpollEvent() {
    return this->epoll_event;
}
