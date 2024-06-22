#include "config.hpp"

void Config::init(int argc, char* argv[]) {
    // todo: parse arguments
    this->port = 8080;
}

int Config::getPort() {
    return this->port;
}

Log::LogLevel Config::getLogLevel() {
    return Log::LogLevel::INFO;
}
