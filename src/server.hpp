#include "config.hpp"
#include "logger.hpp"

class Server {
    public:
        Server(int argc, char* argv[]);
        void start();
    private:
        Config config;
        bool initialized;
        Log::Logger logger;
        int socket_fd;

        void listenLocal();
};
