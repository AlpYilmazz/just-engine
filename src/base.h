#pragma once

#include "raylib.h"
#include "raymath.h"

#include "core.h"

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
        is_eq &= (e1[i] == e2[i]);
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
    PANIC("Incorrect AnchorType!\n");
}

static inline Anchor make_custom_anchor(Vector2 origin) {
    return (Anchor) { .origin = origin };
}

typedef struct {
    Vector2 translation;
    Vector2 scale;
} SpaceShift;

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
    union {
        struct {
            uint32 width;
            uint32 height;
        };
        struct {
            uint32 x;
            uint32 y;
        };
    };
} URectSize;

static inline Rectangle into_rectangle(Vector2 position, RectSize size) {
    return (Rectangle) {
        .x = position.x,
        .y = position.y,
        .width = size.width,
        .height = size.height,
    };
}

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

static inline Vector2 find_rectangle_position(Anchor anchor, Vector2 top_left, RectSize size) {
    return Vector2Add(
        top_left,
        Vector2Multiply(anchor.origin, size.as_vec)
    );
}

static inline Vector2 find_rectangle_position_rect(Anchor anchor, Rectangle rect) {
    Vector2 top_left = {rect.x, rect.y};
    Vector2 size = {rect.width, rect.height};
    return Vector2Add(
        top_left,
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