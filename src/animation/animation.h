#pragma once

#include "base.h"
#include "timer.h"

#include "introspect/introspect.h"

introspect
typedef struct {
    // -- Setup --
    URectSize texture_offset;
    URectSize sprite_size;
    uint32 rows;
    uint32 cols;
    uint32 frame_count;
    // -- State --
    StepTimer timer;
    uint32 current_frame;
    bool finished;
} SpriteSheetAnimationState;

SpriteSheetAnimationState new_animation_state_row(
    URectSize sprite_size,
    uint32 frame_count
);
SpriteSheetAnimationState new_animation_state_grid(
    URectSize sprite_size,
    uint32 rows,
    uint32 cols,
    uint32 frame_count
);
void reset_animation_state(SpriteSheetAnimationState* state);
void tick_animation_state(SpriteSheetAnimationState* state);
void tick_back_animation_state(SpriteSheetAnimationState* state);
Rectangle animation_state_current_frame(SpriteSheetAnimationState* state);
bool animation_is_finished(SpriteSheetAnimationState* state);