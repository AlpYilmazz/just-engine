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
AnimationCurve animation_curve_step(float32 step_cutoff);
AnimationCurve animation_curve_eased(EaseFunction ease_function);

float32 eval_animation_curve(AnimationCurve curve, float32 progress);

typedef enum {
    TWEEN_REPEATING = 0,
    TWEEN_NONREPEATING,
} TweenMode;

typedef enum {
    TWEEN_STARTOVER = 0,
    TWEEN_MIRRORED,
} TweenEndBehaviour;

typedef struct {
    TweenMode mode;
    TweenEndBehaviour on_end;
    AnimationCurve curve;
    float32 duration;
    // --
    float32 elapsed;
    int32 direction; // -1 | 1
} TweenState;

float32 tween_state_tick(TweenState* tween, float32 delta_time);

typedef struct {
    TweenState state;
    Vector2 start;
    Vector2 end;
} Tween_Vector2;

Vector2 vector2_interpolate(Vector2 start, Vector2 end, float32 progress);
Vector2 Vector2__tween_tick(Tween_Vector2* tween, float32 delta_time);

#define Tween(TYPE) Tween_##TYPE
#define tween_tick(TYPE) TYPE##__tween_tick

#define __DECLARE__TWEEN(TYPE) \
    typedef struct { \
        TweenState state; \
        TYPE start; \
        TYPE end; \
    } Tween_##TYPE; \
    TYPE TYPE##__tween_tick(Tween_##TYPE* tween, float32 delta_time);

#define __IMPL_____TWEEN(TYPE, InterpolateFn) \
    TYPE TYPE##__tween_tick(Tween_##TYPE* tween, float32 delta_time) { \
        float32 progress_out = tween_state_tick(&tween->state, delta_time); \
        return (InterpolateFn)(tween->start, tween->end, progress_out); \
    }