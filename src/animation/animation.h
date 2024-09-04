#pragma once

#include "base.h"
#include "assets/asset.h"

typedef enum {
    Timer_NonRepeating,
    Timer_Repeating,
} TimerMode;

typedef struct {
    TimerMode mode;
    float time_setup;
    float time_elapsed;
    bool finished;
} Timer;

Timer new_timer(float setup_secs, TimerMode mode);
void reset_timer(Timer* timer);
void tick_timer(Timer* timer, float delta_time_seconds);
bool timer_is_finished(Timer* timer);

typedef struct {
    TimerMode mode;
    int checkpoint_count;
    int index;
    float* checkpoints;
    float time_elapsed;
    bool pulsed;
    bool finished;
} SequenceTimer;

SequenceTimer new_sequence_timer(float* checkpoints, int count, TimerMode mode);
SequenceTimer new_sequence_timer_evenly_spaced(float time_between, int count, TimerMode mode);
void reset_sequence_timer(SequenceTimer* stimer);
void tick_sequence_timer(SequenceTimer* stimer, float delta_time_seconds);
bool sequence_timer_has_pulsed(SequenceTimer* stimer);
bool sequence_timer_is_finished(SequenceTimer* stimer);

typedef struct {
    SequenceTimer timer;
    TextureHandle* textures;
    int texture_count;
    int current_texture_ind;
} SpriteAnimation;

SpriteAnimation new_sprite_animation(SequenceTimer timer, TextureHandle* textures, int texture_count);
void tick_animation_timer(SpriteAnimation* anim, float delta_time_seconds);
TextureHandle get_current_texture(SpriteAnimation* anim);

typedef struct {
    SequenceTimer timer;
    TextureHandle sprite_sheet_texture;
    Vector2 sprite_size;
    int rows;
    int cols;
    int count;
    int current_sprite_ind;
} SpriteSheetAnimation;

SpriteSheetAnimation new_sprite_sheet_animation(
    SequenceTimer timer,
    TextureHandle sprite_sheet_texture,
    Vector2 sprite_size,
    int rows,
    int cols,
    int count
);
SpriteSheetAnimation new_sprite_sheet_animation_single_row(
    SequenceTimer timer,
    TextureHandle sprite_sheet_texture,
    Vector2 sprite_size,
    int count
);
SpriteSheetAnimation new_sprite_sheet_animation_single_row_even_timer(
    TextureHandle sprite_sheet_texture,
    Vector2 sprite_size,
    int count,
    float time_between,
    TimerMode mode
);
void reset_sprite_sheet_animation(SpriteSheetAnimation* anim);
void tick_sprite_sheet_animation_timer(SpriteSheetAnimation* anim, float delta_time_seconds);

typedef struct {
    TextureHandle texture_handle;
    Rectangle sprite;
} SpriteSheetSprite;

SpriteSheetSprite sprite_sheet_get_current_sprite(SpriteSheetAnimation* anim);

typedef struct {
    RectSize sprite_size;
    uint32 frame_count;
    uint32 current;
} FrameSpriteSheetAnimation;

/**
 * Requires single row sprite sheet
 * With frame_count number of frames
 */
FrameSpriteSheetAnimation new_frame_sprite_sheet_animation(
    RectSize sprite_size,
    uint32 frame_count
);
void reset_frame_sprite_sheet_animation(FrameSpriteSheetAnimation* anim);
void tick_frame_sprite_sheet_animation(FrameSpriteSheetAnimation* anim);
void tick_back_frame_sprite_sheet_animation(FrameSpriteSheetAnimation* anim);
Rectangle sprite_sheet_get_current_frame(FrameSpriteSheetAnimation* anim);