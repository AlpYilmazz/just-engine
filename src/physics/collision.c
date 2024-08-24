#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"

#include "base.h"

#include "collision.h"

float32 just_engine_collider_dist_circle_circle(CircleCollider c1, CircleCollider c2) {
    return Vector2Distance(c1.center, c2.center) - c1.radius - c2.radius;
}

bool just_engine_check_collision_circle_circle(CircleCollider c1, CircleCollider c2) {
    float32 radius_sum = c1.radius + c2.radius;
    return Vector2DistanceSqr(c1.center, c2.center) <= radius_sum * radius_sum + EPSILON;
}

bool just_engine_check_collision_aabb_aabb(AABBCollider r1, AABBCollider r2) {
    return r1.x_left <= r2.x_right &&
          r2.x_left <= r1.x_right &&
          r1.y_top <= r2.y_bottom &&
          r2.y_top <= r1.y_bottom;
}

bool just_engine_check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist) {
    float32 rad_sqr = c1.radius * c1.radius;
    Vector2 center = Vector2Subtract(c1.center, ray.position);
    float32 dot = Vector2DotProduct(ray.direction, center);
    
    if (dot <= 0.0) {
        return Vector2LengthSqr(center) <= rad_sqr;
    }
    if (dot >= max_dist) {
        Vector2 closest_point = Vector2Scale(ray.direction, max_dist);
        return Vector2DistanceSqr(center, closest_point) <= rad_sqr;
    }

    Vector2 proj = Vector2Scale(ray.direction, dot);
    Vector2 perp = Vector2Subtract(center, proj);
    return Vector2LengthSqr(perp) <= rad_sqr;
}