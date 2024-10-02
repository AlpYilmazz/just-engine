#pragma once

typedef enum {
    LOG_LEVEL_ALL = 0,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE,
} LogLevel;

void SET_LOG_LEVEL(LogLevel log_level);

void JUST_LOG_TRACE(const char* format, ...);
void JUST_LOG_DEBUG(const char* format, ...);
void JUST_LOG_INFO(const char* format, ...);
void JUST_LOG_WARN(const char* format, ...);
void JUST_LOG_ERROR(const char* format, ...);
void JUST_LOG_PANIC(const char* format, ...);