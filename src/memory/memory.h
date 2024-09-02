#pragma once

#include "base.h"

#define BUMP_ALLOCATOR_DEFAULT_SIZE 10000

typedef struct {
    byte* base;
    byte* cursor;
    uint32 total_size_in_bytes;
} BumpAllocator;

typedef BumpAllocator TemporaryStorage;

BumpAllocator make_bump_allocator_with_size(uint32 size_in_bytes);
BumpAllocator make_bump_allocator();
void free_bump_allocator(BumpAllocator* bump_allocator);
void reset_bump_allocator(BumpAllocator* bump_allocator);
void* bump_alloc(BumpAllocator* bump_allocator, uint32 size_in_bytes);
