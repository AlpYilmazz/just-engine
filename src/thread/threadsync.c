#include <windows.h>

#include "justcstd.h"
#include "core.h"

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

typedef struct {
    CRITICAL_SECTION csect;
} CriticalSection;

CriticalSection* alloc_create_critical_section() {
    CriticalSection* critical_section = NULL;

    CRITICAL_SECTION csect_win;
    InitializeCriticalSection(&csect_win);
    critical_section = std_malloc(sizeof(CriticalSection));
    critical_section->csect = csect_win;

    return critical_section;
}

void enter_critical_section(CriticalSection* critical_section) {
    EnterCriticalSection(&critical_section->csect);
}

void leave_critical_section(CriticalSection* critical_section) {
    LeaveCriticalSection(&critical_section->csect);
}

void delete_critical_section(CriticalSection* critical_section) {
    DeleteCriticalSection(&critical_section->csect);
    std_free(critical_section);
}

typedef struct {
    CONDITION_VARIABLE condvar;
} ConditionVariable;

ConditionVariable* alloc_create_condition_variable() {
    ConditionVariable* condition_variable = NULL;

    CONDITION_VARIABLE condvar_win;
    InitializeConditionVariable(&condvar_win);
    condition_variable = std_malloc(sizeof(ConditionVariable));
    condition_variable->condvar = condvar_win;

    return condition_variable;
}

bool sleep_condition_variable_srw_shared(ConditionVariable* condition_variable, SRWLock* srwlock, uint32 timeout) {
    return SleepConditionVariableSRW(&condition_variable->condvar, &srwlock->lock, timeout, CONDITION_VARIABLE_LOCKMODE_SHARED);
}

bool sleep_condition_variable_srw_exclusive(ConditionVariable* condition_variable, SRWLock* srwlock, uint32 timeout) {
    return SleepConditionVariableSRW(&condition_variable->condvar, &srwlock->lock, timeout, 0);
}

bool sleep_condition_variable_cs(ConditionVariable* condition_variable, CriticalSection* critical_section, uint32 timeout) {
    return SleepConditionVariableCS(&condition_variable->condvar, &critical_section->csect, timeout);
}

void wake_condition_variable(ConditionVariable* condition_variable) {
    WakeConditionVariable(&condition_variable->condvar);
}

void wake_all_condition_variable(ConditionVariable* condition_variable) {
    WakeAllConditionVariable(&condition_variable->condvar);
}
