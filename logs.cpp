#include "stdio.h"
#include "errno.h"
#include "logs.hpp"

void _log(LOG_LEVEL level, const char* message) {
    if (_currentLevel != OFF && level >= _currentLevel) {
        printf("LOG %s: %s\n", LOG_LEVEL_STRING[level], message);
    }
}

void log_debug(const char* message) {
    _log(DEBUG, message);
}

void log_error(const char* message) {
    if (_currentLevel != OFF && ERROR >= _currentLevel) {
        printf("LOG %s code: %d: %s\n", LOG_LEVEL_STRING[ERROR], errno, message);
    }
}
