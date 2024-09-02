#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"

#include "base.h"

#include "collision.h"

float32 just_engine_collider_dist_circle_circle(CircleCollider c1, CircleCollider c2) {
    return Vector2Distance(c1.center, c2.center) - c1.radius - c2.radius;
}

bool just_engine_check_collision_line_line(LineSegmentCollider l1, LineSegmentCollider l2) {
    return CheckCollisionLines(l1.start, l1.end, l2.start, l2.end, NULL);
}

bool just_engine_check_collision_line_circle(LineSegmentCollider l1, CircleCollider c2) {
    Vector2 dir = Vector2Subtract(l1.end, l1.start);
    Ray2 ray = {
        .position = l1.start,
        .direction = Vector2Normalize(dir),
    };
    return just_engine_check_rayhit_circle(ray, c2, Vector2Length(dir));
}

bool just_engine_check_point_inside_aabb(AABBCollider a1, Vector2 p) {
    return a1.x_left <= p.x && p.x <= a1.x_right
        && a1.y_top <= p.y && p.y <= a1.y_bottom;
}

bool just_engine_check_collision_line_aabb(LineSegmentCollider l1, AABBCollider a1) {
    bool lstart_inside = just_engine_check_point_inside_aabb(a1, l1.start);
    bool lend_inside = just_engine_check_point_inside_aabb(a1, l1.end);

    return lstart_inside || lend_inside;

    // LineSegmentCollider c1_top_side = {
    //     .start = { c1.x_left, c1.y_top },
    //     .end = { c1.x_right, c1.y_top },
    // };
    // LineSegmentCollider c1_bottom_side = {
    //     .start = { c1.x_left, c1.y_bottom },
    //     .end = { c1.x_right, c1.y_bottom },
    // };
    // LineSegmentCollider c1_left_side = {
    //     .start = { c1.x_left, c1.y_top },
    //     .end = { c1.x_left, c1.y_bottom },
    // };
    // LineSegmentCollider c1_rigth_side = {
    //     .start = { c1.x_right, c1.y_top },
    //     .end = { c1.x_right, c1.y_bottom },
    // };

    // return just_engine_check_collision_line_line(l1, c1_top_side)
    //     || just_engine_check_collision_line_line(l1, c1_bottom_side)
    //     || just_engine_check_collision_line_line(l1, c1_left_side)
    //     || just_engine_check_collision_line_line(l1, c1_rigth_side)
    //     || just_engine_check_line_inside_aabb(l1, c1);
}

bool just_engine_check_collision_circle_circle(CircleCollider c1, CircleCollider c2) {
    float32 radius_sum = c1.radius + c2.radius;
    return Vector2DistanceSqr(c1.center, c2.center) <= radius_sum * radius_sum;
}

bool just_engine_check_collision_circle_aabb(CircleCollider c1, AABBCollider a2) {
    float32 aabb_x_bound = (a2.x_right - a2.x_left)/2.0;
    float32 aabb_y_bound = (a2.y_top - a2.y_bottom)/2.0;

    Vector2 aabb_center = {
        .x = a2.x_left + aabb_x_bound,
        .y = a2.y_bottom + aabb_y_bound,
    };

    Vector2 distance = Vector2Subtract(c1.center, aabb_center);
    Vector2 clamp_dist = {
        .x = Clamp(distance.x, -aabb_x_bound, aabb_x_bound),
        .y = Clamp(distance.y, -aabb_y_bound, aabb_y_bound),
    };
    Vector2 closest_point = Vector2Add(aabb_center, clamp_dist);

    return Vector2DistanceSqr(c1.center, closest_point) <= c1.radius * c1.radius;
}

bool just_engine_check_collision_aabb_aabb(AABBCollider a1, AABBCollider a2) {
    return a1.x_left <= a2.x_right &&
          a2.x_left <= a1.x_right &&
          a1.y_top <= a2.y_bottom &&
          a2.y_top <= a1.y_bottom;
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

bool just_engine_check_rayhit_aabb(Ray2 ray, AABBCollider a1, float32 max_dist) {
    LineSegmentCollider ray_line = {
        .start = ray.position,
        .end = Vector2Add(ray.position, Vector2Scale(ray.direction, max_dist)),
    };

    return just_engine_check_collision_line_aabb(ray_line, a1);
}