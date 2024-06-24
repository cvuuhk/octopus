#pragma once
#include <sys/epoll.h>

#include "logger.hpp"

class Server;

class Event {
    public:
        virtual void handleEvent() = 0;
        virtual int getFd() = 0;
        virtual int getMask() = 0;
        virtual void setMask(int mask) = 0;
        virtual struct epoll_event* getEpollEvent() = 0;
        virtual ~Event() { delete epoll_event; }
    protected:
        int fd;
        int mask;
        struct epoll_event* epoll_event;
        Server* server;
};

class AcceptEvent : public Event {
    public:
        AcceptEvent(int fd, Server* server);
        void handleEvent() override;
        int getFd() override;
        int getMask() override;
        void setMask(int mask) override;
        struct epoll_event* getEpollEvent() override;
    private:
        Log::Logger logger;
};

class ReadEvent : public Event {
    public:
        ReadEvent(int fd);
        void handleEvent() override;
        int getFd() override;
        int getMask() override;
        void setMask(int mask) override;
        struct epoll_event* getEpollEvent() override;
};
