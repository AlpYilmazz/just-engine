#pragma once

#include <process.h>

#include "task.h"

typedef struct {
    unsigned int id;
    uintptr_t handle;
} Thread;

Thread thread_spawn(ThreadEntry entry);
void end_thread(unsigned int return_code);
void thread_join(Thread thread);