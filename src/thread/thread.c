#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>

#include "task.h"

#include "thread.h"

// Thread

Thread thread_spawn(ThreadHandlerFn handler, ThreadArgVoid* arg) {
    unsigned int id;
    uintptr_t handle = _beginthreadex( // NATIVE CODE
        NULL,                       // default security attributes
        0,                          // use default stack size  
        handler,                    // thread function name
        arg,                        // argument to thread function 
        0,                          // use default creation flags 
        &id                         // returns the thread identifier 
    );

    return (Thread) {
        .id = id,
        .handle = handle,
    };
}

Thread thread_spawn_with(ThreadEntry entry) {
    return thread_spawn(entry.handler, entry.arg);
}

void end_thread(uint32 return_code) {
    _endthreadex(return_code);
}

void thread_join(Thread thread) {
    WaitForSingleObject((HANDLE) thread.handle, INFINITE);
    CloseHandle((HANDLE) thread.handle);
}

bool thread_try_join(Thread thread) {
    DWORD retval = WaitForSingleObject((HANDLE) thread.handle, 1);
    if (retval != WAIT_OBJECT_0) {
        return false;
    }
    CloseHandle((HANDLE) thread.handle);
    return true;
}

// void issue_interrupt_apc() {
//     bool success = QueueUserAPC(
//         interrupt_network_wait,
//         NETWORK_THREAD,
//         0
//     );
//     if (!success) {
//         int32 err = GetLastError();
//         // TODO: handle err
//         JUST_LOG_ERROR("Interrupt APC failed: %d\n", err);
//     }
// }