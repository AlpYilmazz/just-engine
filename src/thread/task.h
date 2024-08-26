#pragma once

typedef void TaskArgVoid;

typedef struct {
    unsigned int (*handler)(TaskArgVoid*);
    TaskArgVoid* arg;
} ThreadEntry;

typedef struct {
    void (*handler)(TaskArgVoid*);
    TaskArgVoid* arg;
} Task;
