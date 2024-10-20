#include <stdlib.h>

#include "base.h"

#include "memory.h"

// Should we have an allocator interface
typedef enum {
    BUMP_ALLOCATOR,
    ARENA_ALLOCATOR,
    HEAP_ALLOCATOR,
} Allocator;
// -----

usize addr_align_up(usize addr, usize align) {
    return (addr + align - 1) & ~(align - 1);
}

void* ptr_align_up(void* ptr, usize align) {
    return (void*) addr_align_up((usize) ptr, align);
}

// BUMP ALLOCATOR

// BumpAllocator

BumpAllocator make_bump_allocator_with_size(usize size) {
    byte* base = malloc(size);
    return (BumpAllocator) {
        .base = base,
        .cursor = base,
        .total_size = size,
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

void* bump_alloc(BumpAllocator* bump_allocator, usize size) {
    void* mem = bump_allocator->cursor;
    bump_allocator->cursor += size;
    return mem;
}

void* bump_alloc_aligned(BumpAllocator* bump_allocator, MemoryLayout layout) {
    void* mem = ptr_align_up(bump_allocator->cursor, layout.alignment);
    bump_allocator->cursor = (byte*)mem + layout.size;
    return mem;
}

// ARENA ALLOCATOR

// ArenaRegion

ArenaRegion* alloc_create_new_arena_region(usize region_size) {
    ArenaRegion* region = malloc(sizeof(ArenaRegion) + region_size);
    region->next_region = NULL;
    region->total_size = region_size;
    region->free_size = region_size;
    region->cursor = region->base;
    return region;
}

void free_arena_region(ArenaRegion* region) {
    free(region);
}

void reset_arena_region(ArenaRegion* region) {
    region->free_size = region->total_size;
    region->cursor = region->base;
}

static inline bool check_free_space_in_arena_region(ArenaRegion* region, usize size) {
    return size <= region->free_size;
}

static inline bool check_aligned_free_space_in_arena_region(ArenaRegion* region, MemoryLayout layout) {
    void* aligned_cursor = ptr_align_up(region->cursor, layout.alignment);
    return ((byte*)aligned_cursor - region->cursor) + layout.size <= region->free_size;
}

void* alloc_from_arena_region(ArenaRegion* region, usize size) {
    void* mem = region->cursor;
    region->free_size -= size;
    region->cursor += size;
    return mem;
}

void* alloc_aligned_from_arena_region(ArenaRegion* region, MemoryLayout layout) {
    void* mem = ptr_align_up(region->cursor, layout.alignment);
    usize used_size = ((byte*)mem - region->cursor) + layout.size;
    region->free_size -= used_size;
    region->cursor += used_size;
    return mem;
}

// ArenaAllocator

ArenaAllocator make_arena_allocator_with_region_size(usize region_size) {
    return (ArenaAllocator) {
        .region_size = region_size,
        .head_region = alloc_create_new_arena_region(region_size),
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

void* arena_alloc(ArenaAllocator* arena_allocator, usize size) {
    ArenaRegion* region = arena_allocator->head_region;

    // Try to find a region with enough space
    while (region != NULL) {
        if (check_free_space_in_arena_region(region, size)) {
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
    return alloc_from_arena_region(region, size);
}

void* arena_alloc_aligned(ArenaAllocator* arena_allocator, MemoryLayout layout) {
    ArenaRegion* region = arena_allocator->head_region;

    // Try to find a region with enough space
    while (region != NULL) {
        if (check_aligned_free_space_in_arena_region(region, layout)) {
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
    return alloc_aligned_from_arena_region(region, layout);
}