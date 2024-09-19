#pragma once

#include "base.h"

#define BUMP_ALLOCATOR_DEFAULT_SIZE (10 * 1024)

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

#define ARENA_ALLOCATOR_DEFAULT_REGION_SIZE (10 * 1024)

typedef struct ArenaRegion {
    struct ArenaRegion* next_region;
    uint32 size_in_bytes;
    uint32 free_size_in_bytes;
    byte* cursor;
    byte base[];
} ArenaRegion;

typedef struct {
    uint32 region_size;
    ArenaRegion* head_region;
} ArenaAllocator;

ArenaAllocator make_arena_allocator_with_region_size(uint32 region_size_in_bytes);
ArenaAllocator make_arena_allocator();
void free_arena_allocator(ArenaAllocator* arena_allocator);
void reset_arena_allocator(ArenaAllocator* arena_allocator);
void* arena_alloc(ArenaAllocator* arena_allocator, uint32 size_in_bytes);