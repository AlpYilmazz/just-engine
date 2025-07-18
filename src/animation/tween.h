#pragma once

#include "base.h"

typedef float32 (*EaseFunction)(float32 p);

float32 ease_quadratic_in(float32 p);
float32 ease_quadratic_out(float32 p);
float32 ease_quadratic_in_out(float32 p);

float32 ease_cubic_in(float32 p);
float32 ease_cubic_out(float32 p);
float32 ease_cubic_in_out(float32 p);

float32 ease_quartic_in(float32 p);
float32 ease_quartic_out(float32 p);
float32 ease_quartic_in_out(float32 p);

float32 ease_quintic_in(float32 p);
float32 ease_quintic_out(float32 p);
float32 ease_quintic_in_out(float32 p);

float32 ease_sine_in(float32 p);
float32 ease_sine_out(float32 p);
float32 ease_sine_in_out(float32 p);

float32 ease_circular_in(float32 p);
float32 ease_circular_out(float32 p);
float32 ease_circular_in_out(float32 p);

float32 ease_exponential_in(float32 p);
float32 ease_exponential_out(float32 p);
float32 ease_exponential_in_out(float32 p);

float32 ease_elastic_in(float32 p);
float32 ease_elastic_out(float32 p);
float32 ease_elastic_in_out(float32 p);

float32 ease_back_in(float32 p);
float32 ease_back_out(float32 p);
float32 ease_back_in_out(float32 p);

float32 ease_bounce_in(float32 p);
float32 ease_bounce_out(float32 p);
float32 ease_bounce_in_out(float32 p);

typedef enum {
    ANIMATION_CURVE_LINEAR = 0,
    ANIMATION_CURVE_DELAY,
    ANIMATION_CURVE_STEP,
    ANIMATION_CURVE_EASED,
} AnimationCurveType;

typedef struct {
    AnimationCurveType type;
    union {
        float32 step_cutoff;
        EaseFunction ease_function;
    };
} AnimationCurve;

AnimationCurve animation_curve_linear();
AnimationCurve animation_curve_delay();
AnimationCurve animation_curve_step(float32 step_cutoff);
AnimationCurve animation_curve_eased(EaseFunction ease_function);

float32 eval_animation_curve(AnimationCurve curve, float32 progress);

typedef enum {
    TWEEN_ONCE = 0,
    TWEEN_REPEAT_STARTOVER,
    TWEEN_REPEAT_MIRRORED,
    // TODO: TWEEN_REPEAT_CURVEMIRRORED
} TweenMode;

typedef struct {
    TweenMode mode;
    AnimationCurve curve;
    float32 duration;
    // --
    float32 elapsed;
    int32 direction; // -1 | 1
} TweenState;

TweenState new_tween_state(TweenMode mode, AnimationCurve curve, float32 duration);
float32 tween_state_tick(TweenState* tween, float32 delta_time);

typedef struct {
    AnimationCurve curve;
    float32 duration;
} TweenSequenceStateSection;

typedef struct {
    usize count;
    usize capacity;
    TweenSequenceStateSection* items;
} TweenSequenceStateSectionList;

typedef struct {
    TweenMode mode;
    usize section;
    TweenSequenceStateSectionList sections;
    // --
    float32 elapsed;
    int32 direction; // -1 | 1
} TweenSequenceState;

TweenSequenceState new_tween_sequence_state(TweenMode mode);

typedef struct {
    usize section;
    float32 progress_out;
} TweenSequenceTickOut;

TweenSequenceTickOut tween_sequence_state_tick(TweenSequenceState* tween, float32 delta_time);

typedef struct {
    Vector2 start;
    Vector2 end;
} TweenLimits_Vector2;

typedef struct {
    usize count;
    usize capacity;
    TweenLimits_Vector2* items;
} TweenLimitsList_Vector2;

typedef struct {
    TweenState state;
    TweenLimits_Vector2 limits;
} Tween_Vector2;

typedef struct {
    TweenSequenceState state;
    TweenLimitsList_Vector2 limits_list;
} TweenSequence_Vector2;

Vector2 vector2_interpolate(Vector2 start, Vector2 end, float32 progress);
Vector2 Vector2__tween_tick(Tween_Vector2* tween, float32 delta_time);
Vector2 Vector2__tween_sequence_tick(TweenSequence_Vector2* tween, float32 delta_time);

// -------------------------------------------------------------------------------------------------------------------

#define Tween(TYPE) Tween_##TYPE
#define TweenSequence(TYPE) TweenSequence_##TYPE
#define tween_tick(TYPE) TYPE##__tween_tick
#define tween_sequence_tick(TYPE) TYPE##__tween_sequence_tick

// -------------------------------------------------------------------------------------------------------------------

#define __DECLARE__TWEEN(TYPE) \
\
    typedef struct { \
        TYPE start; \
        TYPE end; \
    } TweenLimits_##TYPE; \
\
    typedef struct { \
        usize count; \
        usize capacity; \
        TweenLimits_##TYPE* items; \
    } TweenLimitsList_##TYPE; \
\
    typedef struct { \
        TweenState state; \
        TweenLimits_##TYPE limits_list; \
    } Tween_##TYPE; \
\
    typedef struct { \
        TweenSequenceState state; \
        TweenLimitsList_##TYPE limits; \
    } TweenSequence_##TYPE; \
\
    TYPE TYPE##__tween_tick(Tween_##TYPE* tween, float32 delta_time); \
    TYPE TYPE##__tween_sequence_tick(TweenSequence_##TYPE* tween, float32 delta_time);

// -------------------------------------------------------------------------------------------------------------------

#define __IMPL_____TWEEN(TYPE, InterpolateFn) \
\
    TYPE TYPE##__tween_tick(Tween_##TYPE* tween, float32 delta_time) { \
        float32 progress_out = tween_state_tick(&tween->state, delta_time); \
        return (InterpolateFn)(tween->limits.start, tween->limits.end, progress_out); \
    } \
    TYPE TYPE##__tween_sequence_tick(Tween_##TYPE* tween, float32 delta_time) { \
        TweenSequenceTickOut tick_out = tween_sequence_state_tick(&tween->state, delta_time); \
        TweenLimits_##TYPE limits = tween->limits_list.items[tick_out.section]; \
        return (InterpolateFn)(limits.start, limits.end, tick_out.progress_out); \
    }

// -------------------------------------------------------------------------------------------------------------------
