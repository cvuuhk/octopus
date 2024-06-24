#include <sys/socket.h> // socket, bind, accept, setsockopt
#include <netinet/in.h> // sockaddr_in
#include <strings.h> // bzero, memset
#include <arpa/inet.h> // inet_pton
#include <cstring>
#include <sys/epoll.h>
#include <string>

#include "config.hpp"
#include "server.hpp"
#include "logger.hpp"
#include "event.hpp"

Server::Server(int argc, char* argv[]) {
     this->config.init(argc, argv);
     this->initialized = true;
     this->logger = Log::Logger(this->config.getLogLevel());
     this->epfd = epoll_create(1);
     if(this->epfd == -1) {
         logger.error("error when create epfd: " + std::string(strerror(errno)));
         exit(1);
     }
}

void Server::start() {
    if (!this->initialized) {
        logger.info("server use default config");
    }

    listenLocal();
    addEvent(new AcceptEvent(this->socket_fd, this));
    eventLoop();
}

void Server::addEvent(Event* event) {
    Event* savedEvent = this->events[event->getFd()];

    if (savedEvent != nullptr) {
        struct epoll_event* ee = savedEvent->getEpollEvent();
        ee->events = ee->events | event->getMask();
        epoll_ctl(this->epfd, EPOLL_CTL_MOD, event->getFd(), ee);
    } else {
        struct epoll_event* ee = event->getEpollEvent();
        epoll_ctl(this->epfd, EPOLL_CTL_ADD, event->getFd(), ee);
        this->events[event->getFd()] = event;
    }
}

void Server::removeEvent(Event* event) {
    Event* savedEvent = this->events[event->getFd()];
    if(savedEvent == nullptr) return;

    savedEvent->setMask(savedEvent->getMask() & (~event->getMask()));
    if(savedEvent->getMask() != 0) {
        epoll_ctl(this->epfd, EPOLL_CTL_MOD, event->getFd(), savedEvent->getEpollEvent());
    } else {
        epoll_ctl(this->epfd, EPOLL_CTL_DEL, event->getFd(), nullptr);
        this->events[event->getFd()] = nullptr;
        delete savedEvent;
    }
}

void Server::listenLocal() {
    this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socket_fd < 0) {
        logger.error("error when create socket");
        exit(1);
    }

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(this->config.getPort());
    inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);

    int result = bind(this->socket_fd, (struct sockaddr*)&address, sizeof(address));
    if (result < 0) {
        logger.error("error when bind: " + std::string(strerror(errno)));
        exit(1);
    }

    int reuse = 1;
    setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    result = listen(this->socket_fd, 100);
    if (result < 0) {
        logger.error("error when listen");
        exit(1);
    }

    logger.info("server started, listening on port " + std::to_string(this->config.getPort()));
}

void Server::eventLoop() {
    int fd_nums;
    while(true) {
        struct epoll_event events[5];
        fd_nums = epoll_wait(this->epfd, events, 5, -1);

        if (fd_nums == -1) {
            logger.error("error when epoll_wait: " + std::string(strerror(errno)));
            exit(1);
        }

        for (int i=0; i<fd_nums; ++i)
            this->events[events[i].data.fd]->handleEvent();
    }
}
