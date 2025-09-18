#pragma once

// Sync objects are opeque
// because windows.h and raylib.h
// has name collisions

#define INFINITE 0xffffffff

typedef void SRWLock;

SRWLock* alloc_create_srw_lock();
void free_srw_lock(SRWLock* lock);
void srw_lock_acquire_exclusive(SRWLock* lock);
void srw_lock_acquire_shared(SRWLock* lock);
void srw_lock_release_exclusive(SRWLock* lock);
void srw_lock_release_shared(SRWLock* lock);

#define SRW_LOCK_EXCLUSIVE_ZONE(SRW_LOCK, CodeBlock) \
    do {\
        srw_lock_acquire_exclusive(SRW_LOCK);\
        do { CodeBlock; } while(0);\
        srw_lock_release_exclusive(SRW_LOCK);\
    } while (0)

#define SRW_LOCK_SHARED_ZONE(SRW_LOCK, CodeBlock) \
    do {\
        srw_lock_acquire_shared(SRW_LOCK);\
        do { CodeBlock; } while(0);\
        srw_lock_release_shared(SRW_LOCK);\
    } while (0)

typedef void CriticalSection;

CriticalSection* alloc_create_critical_section();
void delete_critical_section(CriticalSection* critical_section);
void enter_critical_section(CriticalSection* critical_section);
void leave_critical_section(CriticalSection* critical_section);

typedef void ConditionVariable;

ConditionVariable* alloc_create_condition_variable();
bool sleep_condition_variable_srw_shared(ConditionVariable* condition_variable, SRWLock* srwlock, uint32 timeout);
bool sleep_condition_variable_srw_exclusive(ConditionVariable* condition_variable, SRWLock* srwlock, uint32 timeout);
bool sleep_condition_variable_cs(ConditionVariable* condition_variable, CriticalSection* critical_section, uint32 timeout);
void wake_condition_variable(ConditionVariable* condition_variable);
void wake_all_condition_variable(ConditionVariable* condition_variable);
