#pragma once

#include "stdlib.h"

#include "raylib.h"
#include "raymath.h"

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

#define PANIC(message, ...) { JUST_LOG_PANIC(message, __VA_ARGS__); exit(EXIT_FAILURE); }

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
    union {
        struct {
            float32 width;
            float32 height;
        };
        Vector2 as_vec;
    };
} RectSize;

typedef struct {
    uint32 width;
    uint32 height;
} URectSize;

static inline Vector2 find_rectangle_top_left(Anchor anchor, Vector2 position, RectSize size) {
    return Vector2Subtract(
        position,
        Vector2Multiply(anchor.origin, size.as_vec)
    );
}

static inline Vector2 find_rectangle_top_left_rect(Anchor anchor, Rectangle rect) {
    Vector2 position = {rect.x, rect.y};
    Vector2 size = {rect.width, rect.height};
    return Vector2Subtract(
        position,
        Vector2Multiply(anchor.origin, size)
    );
}

typedef uint32 BitMask32;

static inline BitMask32 set_bit(BitMask32 mask, uint32 bit) {
    return mask | (1 << bit);
}

static inline BitMask32 unset_bit(BitMask32 mask, uint32 bit) {
    return mask & ((1 << bit) ^ 0xFFFFFFFF);
}

static inline bool check_bit(BitMask32 mask, uint32 bit) {
    return mask & (1 << bit);
}

static inline bool check_overlap(BitMask32 mask1, BitMask32 mask2) {
    return mask1 & mask2;
}

#define PRIMARY_LAYER 1

typedef struct {
    BitMask32 mask;
} Layers;

static inline void set_layer(Layers* layers, uint32 layer) {
    layers->mask = set_bit(layers->mask, layer);
}

static inline void unset_layer(Layers* layers, uint32 layer) {
    layers->mask = unset_bit(layers->mask, layer);
}

static inline bool check_layer(Layers layers, uint32 layer) {
    return check_bit(layers.mask, layer);
}

static inline bool check_layer_overlap(Layers ls1, Layers ls2) {
    return check_overlap(ls1.mask, ls2.mask);
}

static inline Layers on_single_layer(uint32 layer) {
    Layers layers = {0};
    set_layer(&layers, layer);
    return layers;
}

static inline Layers on_primary_layer() {
    return on_single_layer(PRIMARY_LAYER);
}

typedef struct {
    uint32 id;
    uint32 generation;
} ComponentId;

static inline ComponentId new_component_id(uint32 id, uint32 generation) {
    return (ComponentId) { id, generation };
}

#define Vector2_From(val) ((Vector2) {val, val})
#define Vector2_Ones ((Vector2) {1.0, 1.0})
#define Vector2_Unit_X ((Vector2) {1.0, 0.0})
#define Vector2_Unit_Y ((Vector2) {0.0, 1.0})
#define Vector2_Neg_Unit_X ((Vector2) {-1.0, 0.0})
#define Vector2_Neg_Unit_Y ((Vector2) {0.0, -1.0})
#define Vector2_On_X(val) ((Vector2) {val, 0.0})
#define Vector2_On_Y(val) ((Vector2) {0.0, val})
#define Vector2_XX(vec) ((Vector2) {vec.x, vec.x})
#define Vector2_YY(vec) ((Vector2) {vec.y, vec.y})
#define Vector2_YX(vec) ((Vector2) {vec.y, vec.x})

static inline Vector2 vector2_from(float32 val) {
    return (Vector2) {val, val};
}

static inline Vector2 vector2_ones() {
    return (Vector2) {1.0, 1.0};
}

static inline Vector2 vector2_unit_x() {
    return (Vector2) {1.0, 0.0};
}

static inline Vector2 vector2_unit_y() {
    return (Vector2) {0.0, 1.0};
}

static inline Vector2 vector2_neg_unit_x() {
    return (Vector2) {-1.0, 0.0};
}

static inline Vector2 vector2_neg_unit_y() {
    return (Vector2) {0.0, -1.0};
}

static inline Vector2 vector2_on_x(float32 val) {
    return (Vector2) {val, 0.0};
}

static inline Vector2 vector2_on_y(float32 val) {
    return (Vector2) {0.0, val};
}

static inline Vector2 vector2_xx(Vector2 vec) {
    return (Vector2) {vec.x, vec.x};
}

static inline Vector2 vector2_yy(Vector2 vec) {
    return (Vector2) {vec.y, vec.y};
}

static inline Vector2 vector2_yx(Vector2 vec) {
    return (Vector2) {vec.y, vec.x};
}