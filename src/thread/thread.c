#include <windows.h>
#include <process.h>

#include "task.h"

#include "thread.h"

// Thread

Thread thread_spawn(ThreadEntry entry) {
    unsigned int id;
    uintptr_t handle = _beginthreadex( // NATIVE CODE
        NULL,                       // default security attributes
        0,                          // use default stack size  
        entry.handler,              // thread function name
        entry.arg,                  // argument to thread function 
        0,                          // use default creation flags 
        &id                         // returns the thread identifier 
    );

    return (Thread) {
        .id = id,
        .handle = id,
    };
}

void end_thread(unsigned int return_code) {
    _endthreadex(return_code);
}

void thread_join(Thread thread) {
    WaitForSingleObject((HANDLE) thread.handle, INFINITE);
    CloseHandle((HANDLE) thread.handle);
}