#pragma once

#include "event.hpp"
#include "config.hpp"
#include "logger.hpp"

class Server {
    public:
        Server(int argc, char* argv[]);
        void start();
        void addEvent(Event*);
        void removeEvent(Event*, int mask);

        Log::Logger logger;
    private:
        Config config;
        bool initialized;
        int socket_fd;
        int epfd;
        Event* events[10240];

        void listenLocal();
        void eventLoop();
};
