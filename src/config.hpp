#pragma once

#include "logger.hpp"

class Config {
    public:
        void init(int argc, char* argv[]);
        int getPort();
        Log::LogLevel getLogLevel();
    private:
        int port;
};
