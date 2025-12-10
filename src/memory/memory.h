#pragma once

#include "core.h"

#define dynarray_clear(arr) \
    do { \
        (arr).count = 0; \
    } while(0)

#define dynarray_free(arr) \
    do { \
        if ((arr).capacity > 0) { \
            (arr).count = 0; \
            (arr).capacity = 0; \
            std_free((arr).items); \
        } \
    } while(0)

#define dynarray_free_custom(arr, items_field) \
    do { \
        if ((arr).capacity > 0) { \
            (arr).count = 0; \
            (arr).capacity = 0; \
            std_free((arr)items_field); \
        } \
    } while(0)

#define dynarray_free_custom_2(arr, items_field_1, items_field_2) \
    do { \
        if ((arr).capacity > 0) { \
            (arr).count = 0; \
            (arr).capacity = 0; \
            std_free((arr)items_field_1); \
            std_free((arr)items_field_2); \
        } \
    } while(0)

#define dynarray_reserve(arr, reserve_count) \
    do { \
        usize dynarray_reserve__new_capacity = (arr).count + reserve_count; \
        if ((arr).capacity < dynarray_reserve__new_capacity) { \
            (arr).capacity = dynarray_reserve__new_capacity; \
            (arr).items = std_realloc((arr).items, (arr).capacity * sizeof((arr).items[0])); \
        } \
    } while(0)

#define dynarray_reserve_custom(arr, items_field, reserve_count) \
    do { \
        usize dynarray_reserve_custom__new_capacity = (arr).count + reserve_count; \
        if ((arr).capacity < dynarray_reserve_custom__new_capacity) { \
            (arr).capacity = dynarray_reserve_custom__new_capacity; \
            (arr)items_field = std_realloc((arr)items_field, (arr).capacity * sizeof((arr)items_field[0])); \
        } \
    } while(0)

#define dynarray_reserve_custom_2(arr, items_field_1, items_field_2, reserve_count) \
    do { \
        usize dynarray_reserve_custom__new_capacity = (arr).count + reserve_count; \
        if ((arr).capacity < dynarray_reserve_custom__new_capacity) { \
            (arr).capacity = dynarray_reserve_custom__new_capacity; \
            (arr)items_field = std_realloc((arr)items_field_1, (arr).capacity * sizeof((arr)items_field_1[0])); \
            (arr)items_field = std_realloc((arr)items_field_2, (arr).capacity * sizeof((arr)items_field_2[0])); \
        } \
    } while(0)

#define dynarray_push_back(arr, item) \
    do { \
        const uint32 DYNARRAY_INITIAL_CAPACITY = 2; \
        const uint32 DYNARRAY_GROWTH_FACTOR = 2; \
\
        if ((arr).capacity == 0) { \
            (arr).capacity = DYNARRAY_INITIAL_CAPACITY; \
            (arr).items = std_malloc((arr).capacity * sizeof((item))); \
        } \
        else if ((arr).count == (arr).capacity) { \
            (arr).capacity = DYNARRAY_GROWTH_FACTOR * (arr).capacity; \
            (arr).items = std_realloc((arr).items, (arr).capacity * sizeof((item))); \
        } \
        \
        (arr).items[(arr).count] = (item); \
        (arr).count++; \
    } while(0)

#define dynarray_push_back_custom(arr, items_field, item) \
    do { \
        const uint32 DYNARRAY_INITIAL_CAPACITY = 2; \
        const uint32 DYNARRAY_GROWTH_FACTOR = 2; \
\
        if ((arr).capacity == 0) { \
            (arr).capacity = DYNARRAY_INITIAL_CAPACITY; \
            (arr)items_field = std_malloc((arr).capacity * sizeof((item))); \
        } \
        else if ((arr).count == (arr).capacity) { \
            (arr).capacity = DYNARRAY_GROWTH_FACTOR * (arr).capacity; \
            (arr)items_field = std_realloc((arr)items_field, (arr).capacity * sizeof((item))); \
        } \
        \
        (arr)items_field[(arr).count] = (item); \
        (arr).count++; \
    } while(0)


#define dynarray_push_back_custom_2(arr, items_field_1, item_1, items_field_2, item_2) \
    do { \
        const uint32 DYNARRAY_INITIAL_CAPACITY = 2; \
        const uint32 DYNARRAY_GROWTH_FACTOR = 2; \
\
        if ((arr).capacity == 0) { \
            (arr).capacity = DYNARRAY_INITIAL_CAPACITY; \
            (arr)items_field_1 = std_malloc((arr).capacity * sizeof((item_1))); \
            (arr)items_field_2 = std_malloc((arr).capacity * sizeof((item_2))); \
        } \
        else if ((arr).count == (arr).capacity) { \
            (arr).capacity = DYNARRAY_GROWTH_FACTOR * (arr).capacity; \
            (arr)items_field_1 = std_realloc((arr)items_field_1, (arr).capacity * sizeof((item_1))); \
            (arr)items_field_2 = std_realloc((arr)items_field_2, (arr).capacity * sizeof((item_2))); \
        } \
        \
        (arr)items_field_1[(arr).count] = (item_1); \
        (arr)items_field_2[(arr).count] = (item_2); \
        (arr).count++; \
    } while(0)

#define dynarray_push_back_custom_3(arr, items_field_1, item_1, items_field_2, item_2, items_field_3, item_3) \
    do { \
        const uint32 DYNARRAY_INITIAL_CAPACITY = 2; \
        const uint32 DYNARRAY_GROWTH_FACTOR = 2; \
\
        if ((arr).capacity == 0) { \
            (arr).capacity = DYNARRAY_INITIAL_CAPACITY; \
            (arr)items_field_1 = std_malloc((arr).capacity * sizeof((item_1))); \
            (arr)items_field_2 = std_malloc((arr).capacity * sizeof((item_2))); \
            (arr)items_field_3 = std_malloc((arr).capacity * sizeof((item_3))); \
        } \
        else if ((arr).count == (arr).capacity) { \
            (arr).capacity = DYNARRAY_GROWTH_FACTOR * (arr).capacity; \
            (arr)items_field_1 = std_realloc((arr)items_field_1, (arr).capacity * sizeof((item_1))); \
            (arr)items_field_2 = std_realloc((arr)items_field_2, (arr).capacity * sizeof((item_2))); \
            (arr)items_field_3 = std_realloc((arr)items_field_3, (arr).capacity * sizeof((item_3))); \
        } \
        \
        (arr)items_field_1[(arr).count] = (item_1); \
        (arr)items_field_2[(arr).count] = (item_2); \
        (arr)items_field_3[(arr).count] = (item_3); \
        (arr).count++; \
    } while(0)

#define dynarray_insert(arr, index, item) \
    do { \
        usize dynarray_insert__index = MIN(index, (arr).count); \
        dynarray_reserve((arr), 1); \
        for (usize dynarray_insert__i = (arr).count; dynarray_insert__i > dynarray_insert__index; dynarray_insert__i--) { \
            (arr).items[dynarray_insert__i] = (arr).items[dynarray_insert__i - 1]; \
        } \
        (arr).items[dynarray_insert__index] = item; \
        (arr).count++; \
    } while(0)

#define dynarray_insert_custom(arr, items_field, index, item) \
    do { \
        usize dynarray_insert__index = MIN(index, (arr).count); \
        dynarray_reserve_custom((arr), items_field, 1); \
        for (usize dynarray_insert__i = (arr).count; dynarray_insert__i > dynarray_insert__index; dynarray_insert__i--) { \
            (arr)items_field[dynarray_insert__i] = (arr)items_field[dynarray_insert__i - 1]; \
        } \
        (arr)items_field[dynarray_insert__index] = item; \
        (arr).count++; \
    } while(0)

#define dynarray_clone(dst_arr, src_arr) \
    do { \
        usize dynarray_clone__size = sizeof(*(dst_arr).items) * (dst_arr).count; \
        (src_arr).count = (dst_arr).count; \
        (src_arr).capacity = (dst_arr).count; \
        (src_arr).items = (dst_arr).items; \
        (src_arr).items = std_malloc(dynarray_clone__size); \
        std_memcpy((src_arr).items, (dst_arr).items, dynarray_clone__size); \
    } while(0)

#define dynarray_clone_custom(dst_arr, src_arr, items_field) \
    do { \
        usize dynarray_clone_custom__size = sizeof(*(dst_arr)items_field) * (dst_arr).count; \
        (src_arr).count = (dst_arr).count; \
        (src_arr).capacity = (dst_arr).count; \
        (src_arr)items_field = (dst_arr)items_field; \
        (src_arr)items_field = std_malloc(dynarray_clone_custom__size); \
        std_memcpy((src_arr)items_field, (dst_arr)items_field, dynarray_clone_custom__size); \
    } while(0)

static inline Buffer* malloc_buffer(usize size) {
    Buffer* buffer = std_malloc(sizeof(Buffer) + size); // Buffer + [bytes]
    buffer->length = size;
    buffer->bytes = (byte*) (buffer + 1);
    return buffer;
}

static inline FillBuffer* malloc_fillbuffer(usize size) {
    FillBuffer* fillbuffer = std_malloc(sizeof(FillBuffer) + size); // FillBuffer + [bytes]
    fillbuffer->length = size;
    fillbuffer->bytes = (byte*) (fillbuffer + 1);
    fillbuffer->cursor = fillbuffer->bytes;
    return fillbuffer;
}

// -- Allocation --
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