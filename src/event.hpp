#pragma once
#include "logger.hpp"

class Event {
    public:
        virtual void handleEvent() = 0;
        virtual ~Event() {};
};

class AcceptEvent : public Event {
    public:
        AcceptEvent(int socket_fd);
        void handleEvent() override;
        ~AcceptEvent() {}
    private:
        int fd;
        Log::Logger logger;
};

class ReplyEvent : public Event {
    public:
        void handleEvent();
};
