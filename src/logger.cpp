#include "logger.hpp"

Log::Logger::Logger(Log::LogLevel level) {
    this-> logLevel = level;
}

void Log::Logger::debug(const std::string &message) {
    if (this->logLevel <= Log::LogLevel::DEBUG)
        std::cout << "DEBUG: " << message << std::endl;
}

void Log::Logger::info(const std::string &message) {
    if (this->logLevel <= Log::LogLevel::INFO)
        std::cout << "INFO: " << message << std::endl;
}

void Log::Logger::warn(const std::string &message) {
    if (this->logLevel <= Log::LogLevel::WARN)
        std::cout << "WARN: " << message << std::endl;
}

void Log::Logger::error(const std::string &message) {
    if (this->logLevel <= Log::LogLevel::ERROR)
        std::cout << "ERROR: " << message << std::endl;
}
