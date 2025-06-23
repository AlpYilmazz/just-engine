#pragma once

#include "raylib.h"

#include "base.h"

typedef struct {
    Vector2 position;
    Vector2 direction;
} Ray2;

typedef struct {
    Vector2 start;
    Vector2 end;
} LineSegmentCollider;

typedef struct {
    Vector2 center;
    float32 radius;
} CircleCollider;

typedef struct {
    float32 x_left;
    float32 x_right;
    float32 y_top;
    float32 y_bottom;
} AABBCollider;

typedef struct {
    uint32 count; // collider count
    uint32 capacity;
    AABBCollider bounding_box;
    AABBCollider* colliders;
} AABBColliderSet;

// TODO: FreeRectangleCollider: arbitrarily rotated rectangle

AABBColliderSet clone_aabb_collider_set(AABBColliderSet* set);

static inline AABBCollider shift_aabb(SpaceShift shift, AABBCollider aabb) {
    return (AABBCollider) {
        .x_left = shift.translation.x + (aabb.x_left * shift.scale.x),
        .x_right = shift.translation.x + (aabb.x_right * shift.scale.x),
        .y_top = shift.translation.y + (aabb.y_top * shift.scale.y),
        .y_bottom = shift.translation.y + (aabb.y_bottom * shift.scale.y),
    };
}

float32 collider_dist_circle_circle(CircleCollider c1, CircleCollider c2);
bool check_point_inside_aabb(AABBCollider a1, Vector2 p);

bool check_collision_line_line(LineSegmentCollider l1, LineSegmentCollider l2);
bool check_collision_line_circle(LineSegmentCollider l1, CircleCollider c2);
bool check_collision_line_aabb(LineSegmentCollider l1, AABBCollider a2);

bool check_collision_circle_circle(CircleCollider c1, CircleCollider c2);
bool check_collision_circle_aabb(CircleCollider c1, AABBCollider a2);

bool check_collision_aabb_aabb(AABBCollider a1, AABBCollider a2);
bool check_shifted_collision_aabb_aabb(SpaceShift o1, AABBCollider a1, SpaceShift o2, AABBCollider a2);

bool check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist);
bool check_rayhit_aabb(Ray2 ray, AABBCollider a1, float32 max_dist);

bool check_collision_aabb_collider_sets(AABBColliderSet* s1, AABBColliderSet* s2);
bool check_shifted_collision_aabb_collider_sets(SpaceShift o1, AABBColliderSet* s1, SpaceShift o2, AABBColliderSet* s2);
void recalculate_bounding_box(AABBColliderSet* set);
