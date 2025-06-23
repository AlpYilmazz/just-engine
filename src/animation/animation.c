#include "base.h"

#include "animation.h"

// typedef struct {
//     URectSize texture_offset;
//     URectSize sprite_size;
//     uint32 rows;
//     uint32 cols;
//     uint32 frame_count;
//     StepTimer timer;
//     uint32 current_frame;
//     bool finished;
// } SpriteSheetAnimationState;

SpriteSheetAnimationState new_animation_state_row(
    URectSize sprite_size,
    uint32 frame_count
) {
    return (SpriteSheetAnimationState) {
        .texture_offset = {0, 0},
        .sprite_size = sprite_size,
        .rows = 1,
        .cols = frame_count,
        .frame_count = frame_count,
        //
        .timer = new_step_timer(1, Timer_Repeating),
        .current_frame = 0,
        .finished = false,
    };
}

SpriteSheetAnimationState new_animation_state_grid(
    URectSize sprite_size,
    uint32 rows,
    uint32 cols,
    uint32 frame_count
) {
    return (SpriteSheetAnimationState) {
        .texture_offset = {0, 0},
        .sprite_size = sprite_size,
        .rows = rows,
        .cols = cols,
        .frame_count = frame_count,
        //
        .timer = new_step_timer(1, Timer_Repeating),
        .current_frame = 0,
        .finished = false,
    };
}

void reset_animation_state(SpriteSheetAnimationState* state) {
    reset_step_timer(&state->timer);
    state->current_frame = 0;
    state->finished = false;
}

void tick_animation_state(SpriteSheetAnimationState* state) {
    tick_step_timer(&state->timer);
    if (step_timer_is_finished(&state->timer)) {
        state->current_frame = (state->current_frame + 1) % state->frame_count;
        state->finished = !state->current_frame; // if 0
    }
}

void tick_back_animation_state(SpriteSheetAnimationState* state) {
    state->current_frame = (state->current_frame - 1 + state->frame_count) % state->frame_count;
}

Rectangle animation_state_current_frame(SpriteSheetAnimationState* state) {
    uint32 row_i = state->current_frame / state->rows;
    uint32 col_i = state->current_frame % state->rows;

    return (Rectangle) {
        .x = state->texture_offset.x + (state->sprite_size.width * col_i),
        .x = state->texture_offset.y + (state->sprite_size.height * row_i),
        .width = state->sprite_size.width,
        .height = state->sprite_size.height,
    };
}

bool animation_is_finished(SpriteSheetAnimationState* state) {
    if (state->finished) {
        state->finished = false;
        return true;
    }
    return false;
}