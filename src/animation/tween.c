#include "math.h"

#include "animation/tween.h"

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
AnimationCurve animation_curve_delay() {
    return (AnimationCurve) {
        .type = ANIMATION_CURVE_DELAY,
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
    case ANIMATION_CURVE_DELAY:
        return 0.0;
    case ANIMATION_CURVE_LINEAR:
        return progress_clamped;
    case ANIMATION_CURVE_STEP:
        return (progress_clamped < curve.step_cutoff) ? 0.0 : 1.0;
    case ANIMATION_CURVE_EASED:
        return curve.ease_function(progress_clamped);
    }
    PANIC("Unsupported AnimationCurveType");
}

TweenState new_tween_state(TweenMode mode, AnimationCurve curve, float32 duration) {
    return (TweenState) {
        .mode = mode,
        .curve = curve,
        .duration = duration,
        .elapsed = 0,
        .direction = 1,
    };
}

float32 tween_state_tick(TweenState* tween, float32 delta_time) {
    if (tween->mode == TWEEN_ONCE && tween->elapsed > tween->duration) {
        goto EVAL;
    }

    float32 old_elapsed = tween->elapsed;
    tween->elapsed += tween->direction * delta_time;

    switch (tween->mode) {
    case TWEEN_ONCE:
        break;
    case TWEEN_REPEAT_STARTOVER: {
        float32 elapsed = tween->elapsed;
        while (tween->duration < elapsed) {
            elapsed -= tween->duration;
        }
        tween->elapsed = elapsed;
        break;
    }
    case TWEEN_REPEAT_MIRRORED: {
        JUST_LOG_WARN("----- START: %0.2f +(%0.4f) -> %0.2f -----\n", old_elapsed, tween->direction * delta_time, tween->elapsed);
        float32 elapsed = tween->elapsed;
        bool is_end = false;

        if (elapsed < 0) {
            elapsed *= -1;
            tween->direction *= -1;
            is_end = true;
            JUST_LOG_WARN("mirror start: elapsed: %0.2f, direction: %d\n", elapsed, tween->direction);
        }
        else if (tween->duration < elapsed) {
            elapsed -= tween->duration;
            tween->direction *= -1;
            is_end = true;
            JUST_LOG_WARN("mirror end: elapsed: %0.2f, direction: %d\n", elapsed, tween->direction);
        }

        while (elapsed < 0 || tween->duration < elapsed) {
            elapsed -= tween->duration;
            tween->direction *= -1;
            JUST_LOG_WARN("while: elapsed: %0.2f, direction: %d\n", elapsed, tween->direction);
        }

        // tween->elapsed = ((tween->direction == -1 ? 1 : 0) * tween->duration) + (tween->direction * elapsed);
        if (is_end && tween->direction == 1) {
            tween->elapsed = elapsed;
            JUST_LOG_WARN("flip direction(1): %d: elapsed: %0.2f\n", tween->direction, tween->elapsed);
        }
        else if (is_end && tween->direction == -1) {
            tween->elapsed = tween->duration - elapsed;
            JUST_LOG_WARN("flip direction(-1): %d: elapsed: %0.2f\n", tween->direction, tween->elapsed);
        }
        else {
            tween->elapsed = elapsed;
            JUST_LOG_WARN("continue direction: %d: elapsed: %0.2f\n", tween->direction, tween->elapsed);
        }
        JUST_LOG_WARN("result: elapsed: %0.2f, direction: %d\n", tween->elapsed, tween->direction);
        JUST_LOG_WARN("----- END -----\n");
        break;
    }
    default:
        PANIC("Unsupported TweenMode");
    }

    EVAL:
    float32 progress_in = tween->elapsed / tween->duration;
    float32 progress_out = eval_animation_curve(tween->curve, progress_in);

    return progress_out;
}

TweenSequenceState new_tween_sequence_state(TweenMode mode) {
    return (TweenSequenceState) {
        .mode = mode,
        .section = 0,
        .sections = {0},
        .elapsed = 0,
        .direction = 1,
    };
}

TweenSequenceTickOut tween_sequence_state_tick(TweenSequenceState* tween_sequence, float32 delta_time) {
    TweenSequenceStateSection* tween = &tween_sequence->sections.items[tween_sequence->section];
    bool is_first_section = tween_sequence->section == 0;
    bool is_last_section = tween_sequence->section == tween_sequence->sections.count-1;

    if (tween_sequence->mode == TWEEN_ONCE && is_last_section && tween_sequence->elapsed > tween->duration) {
        goto EVAL;
    }

    tween_sequence->elapsed += tween_sequence->direction * delta_time;

    switch (tween_sequence->mode) {
    case TWEEN_ONCE:
        break;
    case TWEEN_REPEAT_STARTOVER: {
        if (tween->duration < tween_sequence->elapsed) {
            usize section = tween_sequence->section;
            float32 elapsed = tween_sequence->elapsed;

            do {
                elapsed -= tween->duration;
                section = (section + 1) % tween_sequence->sections.count;
                tween = &tween_sequence->sections.items[section];
            } while (tween->duration < elapsed);

            tween_sequence->section = section;
            tween_sequence->elapsed = elapsed;
        }
        break;
    }
    case TWEEN_REPEAT_MIRRORED: {
        usize section = tween_sequence->section;
        float32 elapsed = tween_sequence->elapsed;
        bool is_end = false;

        if (elapsed < 0) {
            elapsed *= -1;
            is_end = true;
            if (is_first_section) {
                tween_sequence->direction *= -1;
            }
            else {
                section = (section - 1 + tween_sequence->sections.count) % tween_sequence->sections.count;
                tween = &tween_sequence->sections.items[section];
            }
        }
        else if (tween->duration < elapsed) {
            elapsed -= tween->duration;
            is_end = true;
            if (is_last_section) {
                tween_sequence->direction *= -1;
            }
            else {
                section = (section + 1) % tween_sequence->sections.count;
                tween = &tween_sequence->sections.items[section];
            }
        }

        while (elapsed < 0 || tween->duration < elapsed) {
            elapsed -= tween->duration;
            if (tween_sequence->direction == -1 && is_first_section) {
                tween_sequence->direction *= -1;
            }
            else if (tween_sequence->direction == 1 && is_last_section) {
                tween_sequence->direction *= -1;
            }
            else {
                section = (section + tween_sequence->direction + tween_sequence->sections.count) % tween_sequence->sections.count;
                tween = &tween_sequence->sections.items[section];
                is_first_section = section == 0;
                is_last_section = section == tween_sequence->sections.count;
            }
        }
        
        tween_sequence->section = section;
        if (is_end && tween_sequence->direction == 1) {
            tween_sequence->elapsed = elapsed;
        }
        else if (is_end && tween_sequence->direction == -1) {
            tween_sequence->elapsed = tween->duration - elapsed;
        }
        else {
            tween_sequence->elapsed = elapsed;
        }
        break;
    }
    default:
        PANIC("Unsupported TweenMode");
    }

    EVAL:
    tween = &tween_sequence->sections.items[tween_sequence->section];
    float32 progress_in = tween_sequence->elapsed / tween->duration;
    float32 progress_out = eval_animation_curve(tween->curve, progress_in);

    return (TweenSequenceTickOut) {
        .section = tween_sequence->section,
        .progress_out = progress_out,
    };
}

// Tween_Vector2

Vector2 vector2_interpolate(Vector2 start, Vector2 end, float32 progress) {
    return Vector2Add(
        start,
        Vector2Scale(
            Vector2Subtract(end, start),
            progress
        )
    );
}

Vector2 Vector2__tween_tick(Tween_Vector2* tween, float32 delta_time) {
    float32 progress_out = tween_state_tick(&tween->state, delta_time);
    return vector2_interpolate(tween->limits.start, tween->limits.end, progress_out);
}

Vector2 Vector2__tween_sequence_tick(TweenSequence_Vector2* tween, float32 delta_time) {
    TweenSequenceTickOut tick_out = tween_sequence_state_tick(&tween->state, delta_time);
    TweenLimits_Vector2 limits = tween->limits_list.items[tick_out.section];
    return vector2_interpolate(limits.start, limits.end, tick_out.progress_out);
}