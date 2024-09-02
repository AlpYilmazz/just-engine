#pragma once

#include "raylib.h"

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

typedef     unsigned char           byte;
// typedef     uint8                   bool;

#define STRUCT_ZERO_INIT {0}
#define LAZY_INIT {0}
#define LATER_INIT {0}
#define UNINIT {0}

#define MAX(a, b) ((a >= b) ? a : b)
#define MIN(a, b) ((a <= b) ? a : b)

#define SIGNOF(a) ( (x == 0) ? 0 : ( (x > 0) ? 1 : -1 ) )

static const uint32 ALL_SET_32 = 0b11111111111111111111111111111111;

#define RECTANGLE_NICHE ( \
    (Rectangle) {   \
        .x = *(float32*) &ALL_SET_32, \
        .y = *(float32*) &ALL_SET_32, \
        .width = *(float32*) &ALL_SET_32, \
        .height = *(float32*) &ALL_SET_32, \
    }   \
)

#define BYTEWISE_EQUALS(e1, e2, type)   \
    ( bytewise_equals((byte*)e1, (byte*)e2, sizeof(type)) )

static inline bool bytewise_equals(byte* e1, byte* e2, uint32 count) {
    bool is_eq = 1;
    for (uint32 i = 0; i < count; i++) {
        is_eq &= (e1[i] == e2[2]);
    }
    return is_eq;
}

typedef enum {
    Anchor_Top_Left = 0,    // DEFAULT
    Anchor_Top_Right,
    Anchor_Bottom_Left,
    Anchor_Bottom_Right,

    Anchor_Top_Mid,
    Anchor_Bottom_Mid,
    Anchor_Left_Mid,
    Anchor_Right_Mid,

    Anchor_Center,

    // Anchor_Custom_UV,       // uv coordinate: [0.0, 1.0]
} AnchorType;

typedef struct {
    Vector2 origin;         // uv coordinate: [0.0, 1.0]
} Anchor;                   // {0} -> origin = {0, 0} -> Anchor_Top_Left

static inline Anchor make_anchor(AnchorType type) {
    switch (type) {
    default:
    case Anchor_Top_Left:
        return (Anchor) { .origin = {0, 0} };
    case Anchor_Top_Right:
        return (Anchor) { .origin = {1, 0} };
    case Anchor_Bottom_Left:
        return (Anchor) { .origin = {0, 1} };
    case Anchor_Bottom_Right:
        return (Anchor) { .origin = {1, 1} };

    case Anchor_Top_Mid:
        return (Anchor) { .origin = {0.5, 0} };
    case Anchor_Bottom_Mid:
        return (Anchor) { .origin = {0.5, 1} };
    case Anchor_Left_Mid:
        return (Anchor) { .origin = {0, 0.5} };
    case Anchor_Right_Mid:
        return (Anchor) { .origin = {1, 0.5} };

    case Anchor_Center:
        return (Anchor) { .origin = {0.5, 0.5} };
    }

    // non-reachable, default in the switch
    // TODO: maybe add assert
    return (Anchor) {0};
}

static inline Anchor make_custom_anchor(Vector2 origin) {
    return (Anchor) { .origin = origin };
}

typedef struct {
    uint32 width;
    uint32 height;
} ScreenSize;

typedef struct {
    uint32 id;
} ComponentId;

static inline ComponentId new_component_id(uint32 id) {
    return (ComponentId) { id };
}