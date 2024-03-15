#include <ctime>
#include "logmanager.h"

LogManager::LogManager(std::string &filename) {
    logFile = std::ofstream(filename, std::ios::app);
    if(!logFile.is_open()){
        std::cerr << "Fail to open log file: " << filename << std::endl;
        return;
    }
}

void LogManager::logToFile(const std::string &message) {
    if(!logFile.is_open()){
        std::cerr << "Fail to open log file." << std::endl;
        return;
    }
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localTime);
    logFile << "[" << timeStr << "] " << message << std::endl;
}

LogManager::~LogManager() {
    logFile.close();
}

void LogManager::open(std::string &filename) {
    logFile = std::ofstream(filename, std::ios::app);
    if(!logFile.is_open()){
        std::cerr << "Fail to open log file: " << filename << std::endl;
        return;
    }
}
