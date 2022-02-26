#include "stdio.h"
#include "logs.h"

void _log(LOG_LEVEL level, char* message) {
    if (_currentLevel != OFF && level >= _currentLevel) {
        printf("LOG %s: %s\n", LOG_LEVEL_STRING[level], message);
    }
}

void log_debug(char* message) {
    _log(DEBUG, message);
}

void log_error(char* message) {
    _log(ERROR, message);
}
