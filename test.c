#include "justengine.h"

#define COUNT 7
void reset_test_tweens(Tween_Vector2* tweens) {
    float32 pathlen = 900;
    float32 seplen = 900.0 / (COUNT-1);

    tweens[0] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_ONCE, animation_curve_eased(ease_cubic_in), 2),
    };
    tweens[1] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_STARTOVER, animation_curve_linear(), 5),
    };
    tweens[2] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_STARTOVER, animation_curve_step(0.5), 2),
    };
    tweens[3] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_STARTOVER, animation_curve_eased(ease_elastic_out), 2),
    };
    tweens[4] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_linear(), 2),
    };
    tweens[5] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_step(0.5), 2),
    };
    tweens[6] = (Tween_Vector2) {
        .state = new_tween_state(TWEEN_REPEAT_MIRRORED, animation_curve_eased(ease_elastic_out), 2),
    };

    Vector2 start_i = { 50, 50 };
    Vector2 end_i = { 50, 50 + pathlen };
    for (uint32 i = 0; i < COUNT; i++) {
        tweens[i].limits.start = start_i;
        tweens[i].limits.end = end_i;
        start_i.x += seplen;
        end_i.x += seplen;
    }
}

int main() {
    SET_LOG_LEVEL(LOG_LEVEL_ERROR);
    // SET_LOG_LEVEL(LOG_LEVEL_WARN);
    // SET_LOG_LEVEL(LOG_LEVEL_TRACE);

    InitWindow(1000, 1000, "Test");
    SetTargetFPS(60);

    Tween_Vector2 tweens[COUNT] = {0};
    reset_test_tweens(tweens);

    Vector2 origin = {25, 25};
    Rectangle rects[COUNT] = {0};
    for (uint32 i = 0; i < COUNT; i++) {
        rects[i].width = 50;
        rects[i].height = 50;
    }

    while (!WindowShouldClose()) {
        float32 delta_time = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            reset_test_tweens(tweens);
        }

        for (uint32 i = 0; i < COUNT; i++) {
            Vector2 pos = tween_tick(Vector2)(&tweens[i], delta_time);
            rects[i].x = pos.x;
            rects[i].y = pos.y;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (uint32 i = 0; i < COUNT; i++) {
            DrawRectanglePro(rects[i], origin, 0, RED);
        }

        EndDrawing();
    }


    return 0;
}