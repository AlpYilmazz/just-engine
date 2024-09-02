#include <stdarg.h>
#include <stdio.h>

#include "logging.h"

static LogLevel LOG_LEVEL = LOG_LEVEL_ALL;

void SET_LOG_LEVEL(LogLevel log_level) {
    LOG_LEVEL = log_level;
}

void JUST_LOG_TRACE(const char* format, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_TRACE) {
        va_list args;
        va_start(args, format);
        printf(format, args);
        va_end(args);
    }
}

void JUST_LOG_DEBUG(const char* format, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_DEBUG) {
        va_list args;
        va_start(args, format);
        printf(format, args);
        va_end(args);
    }
}

void JUST_LOG_INFO(const char* format, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_INFO) {
        va_list args;
        va_start(args, format);
        printf(format, args);
        va_end(args);
    }
}

void JUST_LOG_WARN(const char* format, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_WARN) {
        va_list args;
        va_start(args, format);
        printf(format, args);
        va_end(args);
    }
}

void JUST_LOG_ERROR(const char* format, ...) {
    if (LOG_LEVEL <= LOG_LEVEL_ERROR) {
        va_list args;
        va_start(args, format);
        printf(format, args);
        va_end(args);
    }
}