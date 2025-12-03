#pragma once

#include "core.h"
#include "memory/memory.h"

typedef void ThreadArgVoid;
typedef uint32 (*ThreadHandlerFn)(ThreadArgVoid* thread_arg);

typedef struct {
    ThreadHandlerFn handler;
    ThreadArgVoid* arg;
} ThreadEntry;

typedef struct {
    BumpAllocator temp_storage;
} TaskExecutorContext;
typedef void TaskArgVoid;
typedef void (*TaskHandlerFn)(TaskExecutorContext* context, TaskArgVoid* task_arg);

typedef struct {
    TaskHandlerFn handler;
    TaskArgVoid* arg;
} Task;
