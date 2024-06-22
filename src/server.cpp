#include <sys/socket.h> // socket, bind, accept, setsockopt
#include <netinet/in.h> // sockaddr_in
#include <strings.h> // bzero, memset
#include <arpa/inet.h> // inet_pton
#include <cstring>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <string>

#include "config.hpp"
#include "server.hpp"
#include "logger.hpp"
#include "event.hpp"

Server::Server(int argc, char* argv[]) {
     this->config.init(argc, argv);
     this->initialized = true;
     this->logger = Log::Logger(this->config.getLogLevel());
}

void Server::start() {
    if (!this->initialized) {
        logger.info("server use default config");
    }

    listenLocal();

    Event* event = new AcceptEvent(this->socket_fd);
    std::map<int, Event*> m;
    m.insert(std::make_pair(this->socket_fd, event));

    int epfd = epoll_create(1);
    struct epoll_event* ee = new struct epoll_event;
    ee->events = EPOLLIN;
    ee->data.fd = socket_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, this->socket_fd, ee);

    int fd_nums;
    while(true) {
        struct epoll_event events[5];
        fd_nums = epoll_wait(epfd, events, 5, -1);

        if (fd_nums == -1) {
            logger.error("error when epoll_wait: " + std::string(strerror(errno)));
            exit(1);
        }

        for (int i=0; i<fd_nums; ++i) {
            int fd = events[i].data.fd;
            m.at(fd)->handleEvent();
        }
    }

    // struct sockaddr_in accept_addr;
    // socklen_t accept_len = sizeof(accept_addr);
    // while (true) {
    //     int accept_fd = accept(this->socket_fd, (struct sockaddr*)&accept_addr, &accept_len);
    //     if (accept_fd < 0) {
    //         logger.error("error when accept");
    //         exit(1);
    //     }

    //     int flags;
    //     if ((flags = fcntl(accept_fd, F_GETFL)) == -1) {
    //         logger.error("error when fcntl(" + std::to_string(accept_fd) + ", F_GETFL)");
    //     }
    //     if (fcntl(accept_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    //         logger.error("error when set O_NONBLOCK " + std::to_string(accept_fd));
    //     }

    //     int result;
    //     char buffer[100];
    //     for (;;) {
    //         memset(buffer, '\0', 100);
    //         result = recv(accept_fd, buffer, 99, 0);
    //         if (result == -1) {
    //             if (errno == EAGAIN) continue;
    //             logger.error("error when recv: " + std::string(strerror(errno)));
    //             exit(result);
    //         } else if (result == 0) {
    //             logger.info("client closed connection");
    //             close(accept_fd);
    //             break;
    //         } else {
    //             std::cout << buffer << std::flush;
    //         }
    //     }
    // }
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
