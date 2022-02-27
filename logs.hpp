#ifndef LOGS_H
#define LOGS_H

typedef enum {
    ALL,
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    OFF
} LOG_LEVEL;

static const char *LOG_LEVEL_STRING[] = {
    "ALL", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", ""
};

static LOG_LEVEL _currentLevel = DEBUG;

void log_debug(const char* message);

void log_error(const char* message);

#endif