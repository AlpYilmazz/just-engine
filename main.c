
// typedef     unsigned char           uint8;
// typedef     unsigned short          uint16;
// typedef     unsigned int            uint32;
// typedef     unsigned long long      uint64;

// typedef     char                    int8;
// typedef     short                   int16;
// typedef     int                     int32;
// typedef     long long               int64;

// typedef     float                   float32;
// typedef     double                  float64;

// typedef     uint8                   bool;


// #define DECL_EVENT_SYSTEM_BASE(TEvent, t_event)                                                                          \
//     typedef struct {                                                                                                \
//         uint32 count;                                                                                               \
//         uint32 capacity;                                                                                            \
//         TEvent* items;                                                                                              \
//     } EventBuffer_##TEvent;                                                                                       \
//                                                                                                                     \
//     void event_buffer_##t_event##_push_back(EventBuffer_##TEvent* buffer, TEvent item);                           \
//     void event_buffer_##t_event##_push_back_batch(EventBuffer_##TEvent* buffer, TEvent* items, uint32 count);     \
//     void event_buffer_##t_event##_clear(EventBuffer_##TEvent* buffer);                                            \
//                                                                                                                     \
//     typedef struct {                                                                                                \
//         EventBuffer_##TEvent event_buffers[2];                                                                    \
//         uint8 this_frame_ind;                                                                                       \
//     } Events_##TEvent;                                                                                            \
//                                                                                                                     \
//     uint32 just_engine_events_##t_event##_this_frame_buffer_count(Events_##TEvent* events);                           \
//     void just_engine_events_##t_event##_send_single(Events_##TEvent* events, TEvent event);                           \
//     void just_engine_events_##t_event##_send_batch(Events_##TEvent* events, TEvent* event_list, uint32 count);        \
//     void just_engine_events_##t_event##_swap_buffers(Events_##TEvent* events);                                        \
//                                                                                                                     \
//     typedef struct {                                                                                                \
//         uint32 index;                                                                                               \
//         Events_##TEvent* events;                                                                                  \
//     } EventsIter_##TEvent;                                                                                        \
//                                                                                                                     \
//     EventsIter_##TEvent just_engine_events_iter_##t_event##_new(Events_##TEvent* events, uint32 offset);        \
//     EventsIter_##TEvent just_engine_events_iter_##t_event##_new_zero(Events_##TEvent* events);                  \
//     bool just_engine_events_iter_##t_event##_has_next(EventsIter_##TEvent* iter);                                 \
//     TEvent just_engine_events_iter_##t_event##_read_next(EventsIter_##TEvent* iter);                              \

// #define DECL_EVENT_SYSTEM_EXT_CONSUMABLE(TEvent, t_event)                                                       \
//     TEvent just_engine_events_iter_##t_event##_consume_next(EventsIter_##TEvent* iter);                           


// typedef struct {
//     uint32 id;
//     bool consumed;
// } TestEvent;

// DECL_EVENT_SYSTEM_BASE(TestEvent, test_event)
// DECL_EVENT_SYSTEM_EXT_CONSUMABLE(TestEvent, test_event)

#include <stdio.h>

#include "justengine.h"

void test_rayhit() {
    Ray2 ray = {
        .position = {0, 0},
        .direction = {1, 0},
    };

    #define CIRCLE_TEST_CASES 9
    CircleCollider circle_cases[CIRCLE_TEST_CASES] = {
        {
            .center = {-5, 1},
            .radius = 2,
        },
        {
            .center = {-5, 1},
            .radius = 7,
        },
        {
            .center = {-5, 1},
            .radius = 100,
        },

        {
            .center = {5, 3},
            .radius = 2,
        },
        {
            .center = {5, 3},
            .radius = 4,
        },
        {
            .center = {5, 3},
            .radius = 100,
        },

        {
            .center = {15, 1},
            .radius = 2,
        },
        {
            .center = {15, 1},
            .radius = 7,
        },
        {
            .center = {15, 1},
            .radius = 100,
        },
    };

    for (int i = 0; i < CIRCLE_TEST_CASES; i++) {
        CircleCollider circle = circle_cases[i];
        bool rayhit = just_engine_check_rayhit_circle(ray, circle, 10);
        printf("%d - Circle: {%d, %d} (%d)    ->    Rayhit: %s\n"
            , i, (int)circle.center.x, (int)circle.center.y, (int)circle.radius, rayhit ? "true" : "false");
    }
}

int main() {

    InitWindow(1000, 1000, "Test");
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.zoom = 1.0;
    camera.offset = (Vector2) {
        .x = 1000 / 2.0,
        .y = 1000 / 2.0,
    };

    Texture fire_texture = LoadTexture("test-assets\\fire.png");
    Vector2 position = { 300, 300 };
    Vector2 size = { 490 * 0.1, 970 * 0.1 };
    Vector2 origin = {
        .x = size.x * 0.5, // size.x * 0.5,
        .y = size.y, // size.y * 0.5,
    };
    float32 rotation = 0.0;

    while (!WindowShouldClose()) {

        float32 delta_time = GetFrameTime();

        if (!IsKeyDown(KEY_SPACE)) {
            rotation += 20.0 * delta_time;
            rotation -= (rotation > 360.0) ? 360.0 : 0.0;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            rotation = 0.0;
        }

        BeginDrawing();
        ClearBackground(WHITE);
            BeginMode2D(camera);

                DrawTexturePro(
                    fire_texture,
                    (Rectangle) {0, 0, fire_texture.width, fire_texture.height},
                    (Rectangle) {
                        .x = position.x,
                        .y = position.y,
                        .width = size.x,
                        .height = size.y,
                    },
                    origin,
                    rotation,
                    WHITE
                );

                DrawTexturePro(
                    fire_texture,
                    (Rectangle) {0, 0, -fire_texture.width, fire_texture.height},
                    (Rectangle) {
                        .x = -position.x,
                        .y = position.y,
                        .width = size.x,
                        .height = size.y,
                    },
                    (Vector2) {0, 0},
                    0,
                    WHITE
                );

                DrawTexturePro(
                    fire_texture,
                    (Rectangle) {0, 0, fire_texture.width, -fire_texture.height},
                    (Rectangle) {
                        .x = position.x,
                        .y = -position.y,
                        .width = size.x,
                        .height = size.y,
                    },
                    (Vector2) {0, 0},
                    0,
                    WHITE
                );

                DrawTexturePro(
                    fire_texture,
                    (Rectangle) {0, 0, -fire_texture.width, -fire_texture.height},
                    (Rectangle) {
                        .x = -position.x,
                        .y = -position.y,
                        .width = size.x,
                        .height = size.y,
                    },
                    (Vector2) {0, 0},
                    0,
                    WHITE
                );

                DrawCircleV(position, 10, LIME); // tex origin
                DrawCircle(0, 0, 10, RED); // origin

            EndMode2D();
        EndDrawing();

    }

    return 0;
}