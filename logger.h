#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    Logger(const std::string& logFilePath);
    ~Logger();

    void WriteLog(const std::string& logMessage);
    std::string ReadLog();

     std::string ReadLastLines(int numLines);

private:
    std::string logFilePath;
    std::fstream logFile;
    std::mutex fileMutex;
};

extern Logger logger;
