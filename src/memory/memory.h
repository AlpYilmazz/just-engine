#pragma once

#include "base.h"

typedef struct {
    usize size;
    usize alignment;
} MemoryLayout;

#define layoutof(TYPE) ((MemoryLayout) { .size = sizeof(TYPE), .alignment = _Alignof(TYPE)})
#define array_layoutof(TYPE, count) ((MemoryLayout) { .size = count * sizeof(TYPE), .alignment = _Alignof(TYPE)})

usize addr_align_up(usize addr, usize align);
void* ptr_align_up(void* ptr, usize align);

#define BUMP_ALLOCATOR_DEFAULT_SIZE (10 * 1024)

typedef struct {
    byte* base;
    byte* cursor;
    usize total_size;
} BumpAllocator;

typedef BumpAllocator TemporaryStorage;

BumpAllocator make_bump_allocator_with_size(usize size);
BumpAllocator make_bump_allocator();
void free_bump_allocator(BumpAllocator* bump_allocator);
void reset_bump_allocator(BumpAllocator* bump_allocator);
void* bump_alloc(BumpAllocator* bump_allocator, usize size);
void* bump_alloc_aligned(BumpAllocator* bump_allocator, MemoryLayout layout);

#define ARENA_ALLOCATOR_DEFAULT_REGION_SIZE (10 * 1024)

typedef struct ArenaRegion {
    struct ArenaRegion* next_region;
    usize total_size;
    usize free_size;
    byte* cursor;
    byte base[];
} ArenaRegion;

typedef struct {
    usize region_size;
    ArenaRegion* head_region;
} ArenaAllocator;

ArenaAllocator make_arena_allocator_with_region_size(usize region_size);
ArenaAllocator make_arena_allocator();
void free_arena_allocator(ArenaAllocator* arena_allocator);
void reset_arena_allocator(ArenaAllocator* arena_allocator);
void* arena_alloc(ArenaAllocator* arena_allocator, usize size);
void* arena_alloc_aligned(ArenaAllocator* arena_allocator, MemoryLayout layout);