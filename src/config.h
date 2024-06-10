#pragma once

class Config {
    public:
        static Config init(int argc, char *argv[]);

        int get_listen_port();
    private:
        int listen_port;
};
