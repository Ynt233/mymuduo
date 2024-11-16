/*
 * @Author: Ynt
 * @Date: 2024-11-13 10:35:43
 * @LastEditTime: 2024-11-13 12:13:35
 * @Description: Implementations of Logger class
 */
#include <iostream>
#include "Timestamp.h"
#include "Logger.h"

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int level)
{
    logLevel_ = level;
}

void Logger::log(std::string msg)
{
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO]";
        break;
    case WARNING:
        std::cout << "[WARNING]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    case DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;
    }
    std::cout << Timestamp::now().toString() << ": " << msg << std::endl;
}
