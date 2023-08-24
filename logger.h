#pragma once
#include <string>
#include <fstream>
#include <shared_mutex>

class Logger {
public:
    Logger(const std::string& logFilePath);
    ~Logger();

    void WriteLog(const std::string& logMessage);
    std::string ReadLog();

private:
    std::string logFilePath;
    std::fstream logFile;
    mutable std::shared_mutex fileMutex;
};

extern Logger logger;