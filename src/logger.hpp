#pragma once

#include <iostream>

namespace Log {
    enum LogLevel {
        DEBUG = 1, INFO = 2 , WARN = 3, ERROR = 4
    };

    class Logger {
        public:
            Logger(Log::LogLevel level = Log::LogLevel::INFO);
            void debug(const std::string& message);
            void info(const std::string& message);
            void warn(const std::string& message);
            void error(const std::string& message);
        private:
            LogLevel logLevel;
    };
}

