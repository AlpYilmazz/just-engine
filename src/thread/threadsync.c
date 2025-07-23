#include <windows.h>

#include "justcstd.h"

// DO NOT INCLUDE
// Sync objects are opeque
// #include "threadsync.h"

#ifndef PROMISE_SINGLE_THREADED
    #define MULTITHREADED
#endif

typedef struct {
    SRWLOCK lock;
} SRWLock;

SRWLock* alloc_create_srw_lock() {
    SRWLock* lock = NULL;

    #ifdef MULTITHREADED

    SRWLOCK lock_win = SRWLOCK_INIT;
    lock = std_malloc(sizeof(SRWLock));
    lock->lock = lock_win;

    #endif

    return lock;
}

void free_srw_lock(SRWLock* lock) {
    #ifdef MULTITHREADED
    std_free(lock);
    #endif
}

void srw_lock_acquire_exclusive(SRWLock* lock) {
    #ifdef MULTITHREADED
    AcquireSRWLockExclusive(&lock->lock);
    #endif
}

void srw_lock_acquire_shared(SRWLock* lock) {
    #ifdef MULTITHREADED
    AcquireSRWLockShared(&lock->lock);
    #endif
}

void srw_lock_release_exclusive(SRWLock* lock) {
    #ifdef MULTITHREADED
    ReleaseSRWLockExclusive(&lock->lock);
    #endif
}

void srw_lock_release_shared(SRWLock* lock) {
    #ifdef MULTITHREADED
    ReleaseSRWLockShared(&lock->lock);
    #endif
}

// typedef void AwakableSleep;
// typedef struct {
//     SRWLOCK cond_var;
// } AwakableSleep;

// AwakableSleep* new_awakable_sleep();
// void sleep_awakeable(AwakableSleep* awakable);
// void signal_awake(AwakableSleep* awakable);
