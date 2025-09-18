#pragma once

#include <process.h>

#include "task.h"

typedef struct {
    unsigned int id;
    uintptr_t handle;
} Thread;

Thread thread_spawn(ThreadHandlerFn handler, ThreadArgVoid* arg);
Thread thread_spawn_with(ThreadEntry entry);
void end_thread(uint32 return_code);
void thread_join(Thread thread);
bool thread_try_join(Thread thread);