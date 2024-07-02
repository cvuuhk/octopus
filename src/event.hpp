#pragma once
#include <sys/epoll.h>

#include "logger.hpp"

class Server;

class Event {
    public:
        virtual void doRead() = 0;
        virtual void doWrite() = 0;

        virtual int getFd() { return this->fd; }
        virtual int getMask() { return this->mask; }
        virtual void setMask(int mask) { this->mask = mask; if(this->epollEvent != nullptr) { this->epollEvent->events = mask; } }
        virtual struct epoll_event* getEpollEvent() { return this->epollEvent; }
        virtual ~Event() { delete epollEvent; }
    protected:
        int fd;
        int mask;
        struct epoll_event* epollEvent;
        Server* server;
        Log::Logger logger;
};

class AcceptEvent : public Event {
    public:
        AcceptEvent(int fd, Server* server);
        void doRead() override;
        void doWrite() override {}
};

class ClientEvent : public Event {
    public:
        ClientEvent(int fd, Server* server);
        ~ClientEvent();
        void doRead() override;
        void doWrite() override;
    private:
        size_t bufferSize;
        char* buffer;
};
