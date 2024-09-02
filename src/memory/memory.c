#include <stdlib.h>

#include "base.h"

#include "memory.h"

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
