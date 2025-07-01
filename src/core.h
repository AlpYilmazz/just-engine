#pragma once

#include "stdlib.h"
#include "stdbool.h"

#include "logging.h"

typedef     unsigned char           uint8;
typedef     unsigned short          uint16;
typedef     unsigned int            uint32;
typedef     unsigned long long      uint64;

typedef     char                    int8;
typedef     short                   int16;
typedef     int                     int32;
typedef     long long               int64;

typedef     float                   float32;
typedef     double                  float64;

typedef     uint64                  usize;
typedef     unsigned char           byte;
// typedef     uint8                   bool;

#define branchless_if(cond, on_true, on_false) ( ( ((cond) != 0) * (on_true) ) + ( ((cond) == 0) * (on_false) ) )

#define Option(Type) DeclType_Option_##Type
#define Option_None {0}
#define Option_Some(val) { .is_some = true, .value = val, }
#define DECLARE__Option(Type) typedef struct { bool is_some; Type value; } Option(Type);

DECLARE__Option(uint8);
DECLARE__Option(uint16);
DECLARE__Option(uint32);
DECLARE__Option(uint64);
DECLARE__Option(int8);
DECLARE__Option(int16);
DECLARE__Option(int32);
DECLARE__Option(int64);
DECLARE__Option(float32);
DECLARE__Option(float64);
DECLARE__Option(usize);
DECLARE__Option(byte);
DECLARE__Option(char);

#define STRUCT_ZERO_INIT {0}
#define LAZY_INIT {0}
#define LATER_INIT {0}
#define UNINIT {0}

#define MAX(a, b) ((a >= b) ? a : b)
#define MIN(a, b) ((a <= b) ? a : b)

#define SIGNOF(x) ( (x == 0) ? 0 : ( (x > 0) ? 1 : -1 ) )

#define PANIC(...) { JUST_LOG_PANIC(__VA_ARGS__); exit(EXIT_FAILURE); }
#define UNREACHABLE() { JUST_LOG_PANIC("UNREACHABLE: [%s] [%d]\n", __FILE__, __LINE__); exit(EXIT_FAILURE); }

typedef struct {
    uint32 id;
    uint32 generation;
} EntityId;

static inline EntityId new_entity_id(uint32 id, uint32 generation) {
    return (EntityId) { id, generation };
}

// Memory is owned
typedef struct {
    usize length;
    byte* bytes;
} Buffer;

// Memory is only viewed
typedef Buffer BufferSlice;

typedef struct {
    usize length;
    byte* cursor;
    byte* bytes;
} FillBuffer;

static inline BufferSlice buffer_as_slice(Buffer buffer, usize start, usize length) {
    return (BufferSlice) {
        .bytes = buffer.bytes + start,
        .length = length,
    };
}

static inline BufferSlice buffer_into_slice(Buffer buffer) {
    return buffer_as_slice(buffer, 0, buffer.length);
}

static inline usize filled_length(FillBuffer* buffer) {
    return buffer->cursor - buffer->bytes;
}