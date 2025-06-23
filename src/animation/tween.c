#include "math.h"

#include "tween.h"

#define f32_PI      3.14159265358979323846f
#define f32_2PI     6.28318530717958647692f
#define f64_PI      3.1415926535897932384600000000000000f
#define f64_2PI     6.2831853071795864769228676655900576f

float32 ease_quadratic_in(float32 p) {
    return p * p;
}

float32 ease_quadratic_out(float32 p) {
    return -(p * (p - 2.0));
}

float32 ease_quadratic_in_out(float32 p) {
    return (p < 0.5)
        ? (2.0 * p * p)
        : ((-2.0 * p * p) + (4.0 * p) - 1.0);
}


float32 ease_cubic_in(float32 p) {
    return p * p * p;
}

float32 ease_cubic_out(float32 p) {
    float32 f = p - 1.0;
    return (f * f * f) + 1.0;
}

float32 ease_cubic_in_out(float32 p) {
    if (p < 0.5) {
        return 4.0 * p * p * p;
    }
    else {
        float32 f = (2.0 * p) - 2.0;
        return (0.5 * f * f * f) + 1.0;
    }
}


float32 ease_quartic_in(float32 p) {
    return p * p * p * p;
}

float32 ease_quartic_out(float32 p) {
    float32 f = p - 1.0;
    return (f * f * f * (1.0 - p)) + 1.0;
}

float32 ease_quartic_in_out(float32 p) {
    if (p < 0.5) {
        return 8.0 * p * p * p * p;
    }
    else {
        float32 f = p - 1.0;
        return (-8.0 * f * f * f * f) + 1.0;
    }
}


float32 ease_quintic_in(float32 p) {
    return p * p * p * p * p;
}

float32 ease_quintic_out(float32 p) {
    float32 f = p - 1.0;
    return (f * f * f * f * f) + 1.0;
}

float32 ease_quintic_in_out(float32 p) {
    if (p < 0.5) {
        return 16.0 * p * p * p * p * p;
    }
    else {
        float32 f = (2.0 * p) - 2.0;
        return (0.5 * f * f * f * f * f) + 1.0;
    }
}


float32 ease_sine_in(float32 p) {
    return sinf((p - 1.0) * f32_2PI) + 1.0;
}

float32 ease_sine_out(float32 p) {
    return sinf(p * f32_2PI);
}

float32 ease_sine_in_out(float32 p) {
    return 0.5 * (1.0 - cosf(p * f32_PI));
}


float32 ease_circular_in(float32 p) {
    return 1.0 - sqrtf(1.0 - (p * p));
}

float32 ease_circular_out(float32 p) {
    return sqrtf((2.0 - p) * p);
}

float32 ease_circular_in_out(float32 p) {
    if (p < 0.5) {
        return 0.5 * (1.0 - sqrtf(1.0 - 4.0 * (p * p)));
    }
    else {
        return 0.5 * (sqrtf(-((2.0 * p) - 3.0) * ((2.0 * p) - 1.0)) + 1.0);
    }
}


float32 ease_exponential_in(float32 p) {
    if (p <= 0.0) {
        return 0.0;
    }
    else {
        return powf(2.0f, 10.0 * (MIN(p, 1.0) - 1.0));
    }
}

float32 ease_exponential_out(float32 p) {
    if (p >= 1.0) {
        return 1.0;
    }
    else {
        return 1.0 - powf(2.0f, -10.0 * MAX(p, 0.0));
    }
}

float32 ease_exponential_in_out(float32 p) {
    if (p <= 0.0) {
        return 0.0;
    }
    if (p >= 1.0) {
        return 1.0;
    }

    if (p < 0.5)  {
        return 0.5 * powf(2.0f, (20.0 * p) - 10.0);
    }
    else {
        return -0.5 * powf(2.0f, (-20.0 * p) + 10.0) + 1.0;
    }
}


float32 ease_elastic_in(float32 p) {
    return sinf(13.0 * f32_2PI * p) * powf(2.0f, 10.0 * (p - 1.0));
}

float32 ease_elastic_out(float32 p) {
    return sinf(-13.0 * f32_2PI * (p + 1.0)) * powf(2.0f, -10.0 * p) + 1.0;
}

float32 ease_elastic_in_out(float32 p) {
    if (p < 0.5) {
        return 0.5 * sinf(13.0 * f32_2PI * (2.0 * p)) * powf(2.0, 10.0 * ((2.0 * p) - 1.0));
    }
    else {
        return 0.5 * (sinf(-13.0 * f32_2PI * ((2.0 * p - 1.0) + 1.0))
                * powf(2.0, -10.0 * (2.0 * p - 1.0)) + 2.0);
    }
}


float32 ease_back_in(float32 p) {
    return p * p * p - p * sinf(p * f32_PI);
}

float32 ease_back_out(float32 p) {
    float32 f = 1.0 - p;
    return 1.0 - (f * f * f - f * sinf(f * f32_PI));
}

float32 ease_back_in_out(float32 p) {
    if (p < 0.5) {
        float32 f = 2.0 * p;
        return 0.5 * (f * f * f - f * sinf(f * f32_PI));
    }
    else {
        float32 f = 1.0 - (2.0 * p - 1.0);
        return 0.5 * (1.0 - (f * f * f - f * sinf(f * f32_PI))) + 0.5;
    }
}


float32 ease_bounce_in(float32 p) {
    return 1.0 - ease_bounce_out(1.0 - p);
}

float32 ease_bounce_out(float32 p) {
    if (p < 4.0 / 11.0) {
        return (121.0 * p * p) / 16.0;
    }
    else if (p < 8.0 / 11.0) {
        return (363.0 / 40.0 * p * p) - (99.0 / 10.0 * p) + 17.0 / 5.0;
    }
    else if (p < 9.0 / 10.0) {
        return (4356.0 / 361.0 * p * p) - (35442.0 / 1805.0 * p) + 16061.0 / 1805.0;
    }
    else {
        return (54.0 / 5.0 * p * p) - (513.0 / 25.0 * p) + 268.0 / 25.0;
    }
}

float32 ease_bounce_in_out(float32 p) {
    if (p < 0.5) {
        return 0.5 * ease_bounce_in(p * 2.0);
    }
    else {
        return 0.5 * ease_bounce_out(p * 2.0 - 1.0) + 0.5;
    }
}

// -----

AnimationCurve animation_curve_linear() {
    return (AnimationCurve) {
        .type = ANIMATION_CURVE_LINEAR,
    };
}
AnimationCurve animation_curve_step(float32 step_cutoff) {
    return (AnimationCurve) {
        .type = ANIMATION_CURVE_STEP,
        .step_cutoff = step_cutoff,
    };
}
AnimationCurve animation_curve_eased(EaseFunction ease_function) {
    return (AnimationCurve) {
        .type = ANIMATION_CURVE_EASED,
        .ease_function = ease_function,
    };
}

static inline float32 tween_clamp(float32 p) {
    return (p >= 1.0) ? 1.0 : ((p <= 0) ? 0.0 : p);
}

float32 eval_animation_curve(AnimationCurve curve, float32 progress) {
    float32 progress_clamped = tween_clamp(progress);
    switch (curve.type) {
    case ANIMATION_CURVE_LINEAR:
        return progress_clamped;
    case ANIMATION_CURVE_STEP:
        return (curve.step_cutoff < progress_clamped) ? 0.0 : 1.0;
    case ANIMATION_CURVE_EASED:
        return curve.ease_function(progress_clamped);
    }
    PANIC("Unsupported AnimationCurveType");
}