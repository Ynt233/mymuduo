/*
 * @Author: Ynt
 * @Date: 2024-11-13 10:22:16
 * @LastEditTime: 2024-11-14 14:12:17
 * @Description: Define log format
 */
#pragma once
#include <string>
#include "noncopyable.h"

#define LOG_LENGTH 1024
#define LOG_INFO(logMsgFormat, ...) \
    do {                          \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[LOG_LENGTH] = {0}; \
        snprintf(buf, LOG_LENGTH, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while (0);

#define LOG_WARNING(logMsgFormat, ...) \
    do {                          \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(WARNING); \
        char buf[LOG_LENGTH] = {0}; \
        snprintf(buf, LOG_LENGTH, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while (0);

#define LOG_ERROR(logMsgFormat, ...) \
    do {                          \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[LOG_LENGTH] = {0}; \
        snprintf(buf, LOG_LENGTH, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while (0);

#define LOG_FATAL(logMsgFormat, ...) \
    do {                          \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[LOG_LENGTH] = {0}; \
        snprintf(buf, LOG_LENGTH, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
        exit(-1); \
    }while (0);

#ifdef DEBUG_ON
#define LOG_DEBUG(logMsgFormat, ...) \
    do {                          \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(DEBUG); \
        char buf[LOG_LENGTH] = {0}; \
        snprintf(buf, LOG_LENGTH, logMsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    }while (0);
#else
    #define LOG_DEBUG(logMsgFormat, ...)
#endif
// Define log level: INFO ERROR FATAL DEBUG
enum LogLevel
{
    INFO,
    WARNING,
    ERROR,
    FATAL, // core dump
    DEBUG
};

class Logger : noncopyable
{
public:
    // Get instance
    static Logger &instance();

    void setLogLevel(int level);

    void log(std::string msg);

private:
    int logLevel_;
    Logger() {}
};
