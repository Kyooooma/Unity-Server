#ifndef UNITY_SERVER_LOGMANAGER_H
#define UNITY_SERVER_LOGMANAGER_H
#include <fstream>
#include <iostream>

struct LogManager{
    std::ofstream logFile;

    explicit LogManager(std::string &filename);

    LogManager() = default;

    void open(std::string &filename);

    void logToFile(const std::string &message);

    ~LogManager();
};
#endif //UNITY_SERVER_LOGMANAGER_H
