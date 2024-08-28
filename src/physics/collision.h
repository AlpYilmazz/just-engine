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

// TODO: RectangleCollider: arbitrarily rotated rectangle

float32 just_engine_collider_dist_circle_circle(CircleCollider c1, CircleCollider c2);

bool just_engine_check_collision_line_line(LineSegmentCollider l1, LineSegmentCollider l2);
bool just_engine_check_collision_circle_circle(CircleCollider c1, CircleCollider c2);
bool just_engine_check_collision_aabb_aabb(AABBCollider r1, AABBCollider r2);

bool just_engine_check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist);
bool just_engine_check_rayhit_aabb(Ray2 ray, AABBCollider c1, float32 max_dist);