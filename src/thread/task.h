#pragma once

#include "core.h"

typedef void ThreadArgVoid;
typedef uint32 (*ThreadHandlerFn)(ThreadArgVoid*);

typedef struct {
    ThreadHandlerFn handler;
    ThreadArgVoid* arg;
} ThreadEntry;

typedef void TaskArgVoid;
typedef void (*TaskHandlerFn)(TaskArgVoid*);

typedef struct {
    TaskHandlerFn handler;
    TaskArgVoid* arg;
} Task;
