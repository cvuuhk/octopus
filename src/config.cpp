#include "config.h"

Config Config::init(int argc, char **argv) {
    // todo: parse arguments
    Config config = Config();
    config.listen_port = 8080;

    return config;
}

int Config::get_listen_port() {
    return this->listen_port;
}
