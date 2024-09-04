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

// TODO: FreeRectangleCollider: arbitrarily rotated rectangle

float32 just_engine_collider_dist_circle_circle(CircleCollider c1, CircleCollider c2);
bool just_engine_check_point_inside_aabb(AABBCollider a1, Vector2 p);

bool just_engine_check_collision_line_line(LineSegmentCollider l1, LineSegmentCollider l2);
bool just_engine_check_collision_line_circle(LineSegmentCollider l1, CircleCollider c2);
bool just_engine_check_collision_line_aabb(LineSegmentCollider l1, AABBCollider a2);

bool just_engine_check_collision_circle_circle(CircleCollider c1, CircleCollider c2);
bool just_engine_check_collision_circle_aabb(CircleCollider c1, AABBCollider a2);

bool just_engine_check_collision_aabb_aabb(AABBCollider a1, AABBCollider a2);

bool just_engine_check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist);
bool just_engine_check_rayhit_aabb(Ray2 ray, AABBCollider a1, float32 max_dist);

// typedef enum {
//     ColliderType_Line = 0,
//     ColliderType_Circle,
//     ColliderType_AABB,
//     // ColliderType_FreeRectangle,
// } ColliderType;

// typedef struct {
//     ColliderType type;
// } HitBoxHeader;

// typedef struct {
//     HitBoxHeader header;
//     LineSegmentCollider collider;
// } LineSegmentHitBox;

// typedef struct {
//     HitBoxHeader header;
//     CircleCollider collider;
// } CircleHitBox;

// typedef struct {
//     HitBoxHeader header;
//     AABBCollider collider;
// } AABBHitBox;

// bool check_line_hitbox_collision(LineSegmentHitBox* hitbox_1, HitBoxHeader* hb2) {
//     switch(hb2->type) {
//         case ColliderType_Line: {
//             LineSegmentHitBox* hitbox_2 = (void*) hb2;
//             return just_engine_check_collision_line_line(hitbox_1->collider, hitbox_2->collider);
//         }
//         case ColliderType_Circle: {
//             CircleHitBox* hitbox_2 = (void*) hb2;
//             // TODO: impl
//             return just_engine_check_collision_line_circle(hitbox_1->collider, hitbox_2->collider);
//         }
//         case ColliderType_AABB: {
//             AABBHitBox* hitbox_2 = (void*) hb2;
//             // TODO: impl
//             return just_engine_check_collision_line_aabb(hitbox_1->collider, hitbox_2->collider);
//         }
//     }
// }

// bool check_hitbox_collision(HitBoxHeader* hb1, HitBoxHeader* hb2) {
//     switch (hb1->type) {
//     case ColliderType_Line: {
//         LineSegmentHitBox* hitbox_1 = (void*) hb1;
//         return check_line_hitbox_collision(hitbox_1, hb2);
//     }
//     }
// }
