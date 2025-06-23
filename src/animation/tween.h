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
