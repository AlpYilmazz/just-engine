#pragma once

#include "base.h"

#include "introspect/introspect.h"

typedef enum {
    Timer_Repeating = 0,
    Timer_NonRepeating,
} TimerMode;

typedef struct {
    TimerMode mode;
    float32 time_setup;
    float32 time_elapsed;
    bool finished;
} Timer;

Timer new_timer(float32 setup_secs, TimerMode mode);
void reset_timer(Timer* timer);
void tick_timer(Timer* timer, float32 delta_time);
bool timer_is_finished(Timer* timer);

typedef struct {
    TimerMode mode;
    uint32 checkpoint_count;
    float32* checkpoints;
    uint32 current_index;
    float32 time_elapsed;
    bool pulsed;
    bool finished;
} SequenceTimer;

SequenceTimer new_sequence_timer(float32* ref_checkpoints, uint32 count, TimerMode mode);
SequenceTimer new_sequence_timer_evenly_spaced(float32 time_between, uint32 count, TimerMode mode);
void reset_sequence_timer(SequenceTimer* timer);
void tick_sequence_timer(SequenceTimer* timer, float delta_time);
bool sequence_timer_has_pulsed(SequenceTimer* timer);
bool sequence_timer_is_finished(SequenceTimer* timer);

introspect
typedef struct {
    TimerMode mode enum();
    uint32 step_count;
    uint32 current_step;
    bool finished;
} StepTimer;

StepTimer new_step_timer(uint32 count, TimerMode mode);
void reset_step_timer(StepTimer* timer);
void tick_step_timer(StepTimer* timer);
bool step_timer_is_finished(StepTimer* timer);
