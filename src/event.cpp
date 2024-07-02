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
    this->epollEvent = new struct epoll_event;
    this->epollEvent->events = EPOLLIN;
    this->epollEvent->data.fd = fd;
    this->server = server;
    this->logger = Log::Logger();
}

void AcceptEvent::doRead() {
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
    this->server->addEvent(new ClientEvent(accept_fd, this->server));
}

ClientEvent::ClientEvent(int fd, Server* server) {
    this->fd = fd;
    this->mask = EPOLLIN;
    this->epollEvent = new struct epoll_event;
    this->epollEvent->events = EPOLLIN;
    this->epollEvent->data.fd = fd;
    this->server = server;
    this->logger = Log::Logger();
}

ClientEvent::~ClientEvent() {
    if (this->buffer != nullptr) delete [] this->buffer;
    // 导致 double free, EPOLL_CTL_DEL 应该是会自己删除 epoll_event 对象
    // if (this->epollEvent != nullptr) delete this->epollEvent;
}

void ClientEvent::doRead() {
    struct sockaddr_in accept_addr;
    socklen_t accept_len = sizeof(accept_addr);
    // todo: protocol read
    while (true) {
        int result;
        char buffer[100];
        memset(buffer, '\0', 100);
        result = recv(this->fd, buffer, 99, 0);
        if (result == -1) {
            if (errno == EAGAIN) break;
            logger.error("error when recv: " + std::string(strerror(errno)));
            exit(result);
        } else if (result == 0) {
            logger.info("client disconnected");
            this->server->removeEvent(this, EPOLLIN);
            break;
        } else {
            std::cout << buffer << std::flush;
            char* newBuffer = new char[result+1];
            for (int i = 0; i <= result; ++i) newBuffer[i] = buffer[i];
            this->buffer = newBuffer;
            this->bufferSize = result;
            this->mask = this->mask | EPOLLOUT;
            this->epollEvent->events = this->epollEvent->events | EPOLLOUT;
            this->server->addEvent(this);
        }
    }
}

void ClientEvent::doWrite() {
    write(this->fd, this->buffer, this->bufferSize);
    this->server->removeEvent(this, EPOLLOUT);
}
