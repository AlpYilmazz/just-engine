#include <stdlib.h>

#include "base.h"

#include "memory.h"

// Should we have an allocator interface
typedef enum {
    BUMP_ALLOCATOR,
    ARENA_ALLOCATOR,
    HEAP_ALLOCATOR,
} Allocator;

// BUMP ALLOCATOR

// BumpAllocator

BumpAllocator make_bump_allocator_with_size(uint32 size_in_bytes) {
    byte* base = malloc(size_in_bytes);
    return (BumpAllocator) {
        .base = base,
        .cursor = base,
        .total_size_in_bytes = size_in_bytes,
    };
}

BumpAllocator make_bump_allocator() {
    return make_bump_allocator_with_size(BUMP_ALLOCATOR_DEFAULT_SIZE);
}

void free_bump_allocator(BumpAllocator* bump_allocator) {
    free(bump_allocator->base);
}

void reset_bump_allocator(BumpAllocator* bump_allocator) {
    bump_allocator->cursor = bump_allocator->base;
}

void* bump_alloc(BumpAllocator* bump_allocator, uint32 size_in_bytes) {
    void* ptr = bump_allocator->cursor;
    bump_allocator->cursor += size_in_bytes;
    return ptr;
}

// ARENA ALLOCATOR

// ArenaRegion

ArenaRegion* alloc_create_new_arena_region(uint32 region_size_in_bytes) {
    ArenaRegion* region = malloc(sizeof(ArenaRegion) + region_size_in_bytes);
    region->next_region = NULL;
    region->size_in_bytes = region_size_in_bytes;
    region->free_size_in_bytes = region_size_in_bytes;
    region->cursor = region->base;
    return region;
}

void free_arena_region(ArenaRegion* region) {
    free(region);
}

void reset_arena_region(ArenaRegion* region) {
    region->free_size_in_bytes = region->size_in_bytes;
    region->cursor = region->base;
}

void* alloc_from_arena_region(ArenaRegion* region, uint32 size_in_bytes) {
    void* mem = region->cursor;
    region->free_size_in_bytes -= size_in_bytes;
    region->cursor += size_in_bytes;
    return mem;
}

// ArenaAllocator

ArenaAllocator make_arena_allocator_with_region_size(uint32 region_size_in_bytes) {
    return (ArenaAllocator) {
        .region_size = region_size_in_bytes,
        .head_region = alloc_create_new_arena_region(region_size_in_bytes),
    };
}

ArenaAllocator make_arena_allocator() {
    return make_arena_allocator_with_region_size(ARENA_ALLOCATOR_DEFAULT_REGION_SIZE);
}

void free_arena_allocator(ArenaAllocator* arena_allocator) {
    ArenaRegion* region = arena_allocator->head_region;
    while (region != NULL) {
        free_arena_region(region);
        region = region->next_region;
    }
}

void reset_arena_allocator(ArenaAllocator* arena_allocator) {
    ArenaRegion* region = arena_allocator->head_region;
    while (region != NULL) {
        reset_arena_region(region);
        region = region->next_region;
    }
}

void* arena_alloc(ArenaAllocator* arena_allocator, uint32 size_in_bytes) {
    ArenaRegion* region = arena_allocator->head_region;

    // Try to find a region with enough space
    while (region != NULL) {
        if (size_in_bytes <= region->free_size_in_bytes) {
            break;
        }
        region = region->next_region;
    }

    // No suitable region could not be found
    // Create new region at head
    if (region == NULL) {
        ArenaRegion* new_region = alloc_create_new_arena_region(arena_allocator->region_size);
        new_region->next_region = arena_allocator->head_region;
        arena_allocator->head_region = new_region;
        region = new_region;
    }

    // Bump alloc from the region
    return alloc_from_arena_region(region, size_in_bytes);
}

