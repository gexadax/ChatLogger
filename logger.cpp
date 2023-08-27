#include "logger.h"
#include <iostream>
#include <ctime>
#include <vector>
#include <algorithm>
#include <sstream>

Logger::Logger(const std::string& logFilePath) : logFilePath(logFilePath) {
    logFile.open(logFilePath, std::ios::out | std::ios::app);

    if (!logFile.is_open()) {
        std::cerr << "Error: Unable to open log file '" << logFilePath << "' for writing." << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::WriteLog(const std::string& logMessage) {
    std::lock_guard<std::mutex> lock(fileMutex);

    if (logFile.is_open()) {
        time_t now = time(0);
        struct tm currentTime;
        localtime_s(&currentTime, &now);

        char timeStr[80];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &currentTime);

        logFile << "[" << timeStr << "] " << logMessage << std::endl;
        logFile.flush();

    }
    else {
        std::cerr << "Error: Log file is not open." << std::endl;
    }
}

std::string Logger::ReadLog() {
    std::lock_guard<std::mutex> lock(fileMutex);

    if (logFile.is_open()) {
        logFile.seekg(0, std::ios::end);
        std::streampos fileSize = logFile.tellg();

        if (fileSize <= 0) {
            return "";
        }
      
        const int bufferSize = std::min<int>(4096, static_cast<int>(fileSize));
        logFile.seekg(-bufferSize, std::ios::end);
        std::vector<char> buffer(bufferSize);
        logFile.read(&buffer[0], bufferSize);

        std::string logContent(buffer.begin(), buffer.end());
        return logContent;
    }
    else {
        std::cerr << "Error: Log file is not open." << std::endl;
        return "";
    }
}

std::string Logger::ReadLastLines(int numLines) {
    std::lock_guard<std::mutex> lock(fileMutex);

    if (logFile.is_open()) {
        std::vector<std::string> lines;
        std::string line;

        logFile.seekg(0, std::ios::end);
        std::streampos fileSize = logFile.tellg();

        if (fileSize <= 0) {
            return "";
        }

        while (std::getline(logFile, line)) {
            lines.push_back(line);
        }

        int startIndex = std::max(static_cast<int>(lines.size()) - numLines, 0);

        std::ostringstream result;
        for (int i = startIndex; i < lines.size(); ++i) {
            result << lines[i] << "\n";
        }

        return result.str();
    }
    else {
        std::cerr << "Error: Log file is not open." << std::endl;
        return "";
    }
}

