#include "logger.h"
#include <iostream>
#include <ctime>

Logger::Logger(const std::string& logFilePath) : logFilePath(logFilePath) {
    logFile.open(logFilePath, std::ios::out | std::ios::app);
}

Logger::~Logger() {
    logFile.close();
}

void Logger::WriteLog(const std::string& logMessage) {
    std::unique_lock<std::shared_mutex> lock(fileMutex);
    if (logFile.is_open()) {
        time_t now = time(0);
        struct tm currentTime;
        localtime_s(&currentTime, &now);

        char timeStr[80];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &currentTime);

        logFile << "[" << timeStr << "] " << logMessage << std::endl;
    }
}

std::string Logger::ReadLog() {
    std::shared_lock<std::shared_mutex> lock(fileMutex);
    std::string logContent;
    std::string line;
    while (std::getline(logFile, line)) {
        logContent += line + '\n';
    }
    logFile.clear();
    logFile.seekg(0);
    return logContent;
}
