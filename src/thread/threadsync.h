#pragma once

// Sync objects are opeque
// because windows.h and raylib.h
// has name collisions

typedef void SRWLock;

SRWLock* alloc_create_srw_lock();
void free_srw_lock(SRWLock* lock);
void srw_lock_acquire_exclusive(SRWLock* lock);
void srw_lock_acquire_shared(SRWLock* lock);
void srw_lock_release_exclusive(SRWLock* lock);
void srw_lock_release_shared(SRWLock* lock);
