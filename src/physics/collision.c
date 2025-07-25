#include "raylib.h"
#include "raymath.h"

#include "justcstd.h"
#include "base.h"

#include "collision.h"

AABBColliderSet clone_aabb_collider_set(AABBColliderSet* set) {
    usize size = sizeof(*set->colliders) * set->count;

    AABBColliderSet set_clone = *set;
    set_clone.capacity = set->count;
    set_clone.colliders = std_malloc(size);

    std_memcpy(set_clone.colliders, set->colliders, size);

    return set_clone;
}

// -----

float32 collider_dist_circle_circle(CircleCollider c1, CircleCollider c2) {
    return Vector2Distance(c1.center, c2.center) - c1.radius - c2.radius;
}

bool check_collision_line_line(LineSegmentCollider l1, LineSegmentCollider l2) {
    return CheckCollisionLines(l1.start, l1.end, l2.start, l2.end, NULL);
}

bool check_collision_line_circle(LineSegmentCollider l1, CircleCollider c2) {
    Vector2 dir = Vector2Subtract(l1.end, l1.start);
    Ray2 ray = {
        .position = l1.start,
        .direction = Vector2Normalize(dir),
    };
    return check_rayhit_circle(ray, c2, Vector2Length(dir));
}

bool check_point_inside_aabb(AABBCollider a1, Vector2 p) {
    return a1.x_left <= p.x && p.x <= a1.x_right
        && a1.y_top <= p.y && p.y <= a1.y_bottom;
}

bool check_collision_line_aabb(LineSegmentCollider l1, AABBCollider a1) {
    bool lstart_inside = check_point_inside_aabb(a1, l1.start);
    bool lend_inside = check_point_inside_aabb(a1, l1.end);

    if (lstart_inside || lend_inside) {
        return true;
    }

    float32 l1_len = Vector2Distance(l1.start, l1.end);
    Vector2 l1_dir = Vector2Normalize(Vector2Subtract(l1.end, l1.start));
    
    float32 l1_x_min = MIN(l1.start.x, l1.end.x);
    float32 l1_x_max = MAX(l1.start.x, l1.end.x);
    float32 l1_y_min = MIN(l1.start.y, l1.end.y);
    float32 l1_y_max = MAX(l1.start.y, l1.end.y);

    // top line
    float32 y_lines[2] = {a1.y_top, a1.y_bottom};
    for (uint32 i = 0; i < 2; i++) {
        float32 y_line = y_lines[i];

        if (y_line < l1_y_min || l1_y_max < y_line) {
            continue;
        }

        float32 t = (y_line - l1.start.y) / l1_dir.y;
        float32 x = (l1_dir.x * t) + l1.start.x;
        if (a1.x_left <= x && x <= a1.x_right) {
            return true;
        }
    }

    float32 x_lines[2] = {a1.x_left, a1.x_right};
    for (uint32 i = 0; i < 2; i++) {
        float32 x_line = x_lines[i];

        if (x_line < l1_x_min || l1_x_max < x_line) {
            continue;
        }

        float32 t = (x_line - l1.start.x) / l1_dir.x;
        float32 y = (l1_dir.y * t) + l1.start.y;
        if (a1.y_bottom <= y && y <= a1.y_top) {
            return true;
        }
    }

    return false;
}

bool check_collision_circle_circle(CircleCollider c1, CircleCollider c2) {
    float32 radius_sum = c1.radius + c2.radius;
    return Vector2DistanceSqr(c1.center, c2.center) <= radius_sum * radius_sum;
}

bool check_collision_circle_aabb(CircleCollider c1, AABBCollider a2) {
    float32 aabb_x_bound = (a2.x_right - a2.x_left)/2.0;
    float32 aabb_y_bound = (a2.y_bottom - a2.y_top)/2.0;

    Vector2 aabb_center = {
        .x = a2.x_left + aabb_x_bound,
        .y = a2.y_top + aabb_y_bound,
    };

    Vector2 distance = Vector2Subtract(c1.center, aabb_center);
    Vector2 clamp_dist = {
        .x = Clamp(distance.x, -aabb_x_bound, aabb_x_bound),
        .y = Clamp(distance.y, -aabb_y_bound, aabb_y_bound),
    };
    Vector2 closest_point = Vector2Add(aabb_center, clamp_dist);

    return Vector2DistanceSqr(c1.center, closest_point) <= c1.radius * c1.radius;
}

bool check_collision_aabb_aabb(AABBCollider a1, AABBCollider a2) {
    return a1.x_left <= a2.x_right
        && a2.x_left <= a1.x_right
        && a1.y_top <= a2.y_bottom
        && a2.y_top <= a1.y_bottom;
}

bool check_shifted_collision_aabb_aabb(SpaceShift s1, AABBCollider a1, SpaceShift s2, AABBCollider a2) {
    a1 = shift_aabb(s1, a1);
    a2 = shift_aabb(s2, a2);
    return check_collision_aabb_aabb(a1, a2);
}

bool check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist) {
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

bool check_rayhit_aabb(Ray2 ray, AABBCollider a1, float32 max_dist) {
    LineSegmentCollider ray_line = {
        .start = ray.position,
        .end = Vector2Add(ray.position, Vector2Scale(ray.direction, max_dist)),
    };

    return check_collision_line_aabb(ray_line, a1);
}

bool check_collision_aabb_collider_sets(AABBColliderSet* s1, AABBColliderSet* s2) {
    if (s1->count == 0 || s2->count == 0) {
        return false;
    }
    if (check_collision_aabb_aabb(s1->bounding_box, s2->bounding_box)) {
        if (s1->count == 1 && s2->count == 1) {
            return true;
        }
        for (uint32 i1 = 0; i1 < s1->count; i1++) {
            for (uint32 i2 = 0; i2 < s2->count; i2++) {
                if (check_collision_aabb_aabb(s1->colliders[i1], s2->colliders[i2])) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool check_shifted_collision_aabb_collider_sets(SpaceShift o1, AABBColliderSet* s1, SpaceShift o2, AABBColliderSet* s2) {
    if (s1->count == 0 || s2->count == 0) {
        return false;
    }
    if (check_shifted_collision_aabb_aabb(o1, s1->bounding_box, o2, s2->bounding_box)) {
        if (s1->count == 1 && s2->count == 1) {
            return true;
        }
        for (uint32 i1 = 0; i1 < s1->count; i1++) {
            for (uint32 i2 = 0; i2 < s2->count; i2++) {
                if (check_shifted_collision_aabb_aabb(o1, s1->colliders[i1], o2, s2->colliders[i2])) {
                    return true;
                }
            }
        }
    }
    return false;
}

void recalculate_bounding_box(AABBColliderSet* set) {
    if (set->count == 0) {
        return;
    }

    AABBCollider bb = set->colliders[0];
    for (uint32 i = 1; i < set->count; i++) {
        AABBCollider box = set->colliders[i];
        bb.x_left = MIN(box.x_left, bb.x_left);
        bb.x_right = MAX(box.x_right, bb.x_right);
        bb.y_top = MIN(box.y_top, bb.y_top);
        bb.y_bottom = MAX(box.y_bottom, bb.y_bottom);
    }
    set->bounding_box = bb;
}