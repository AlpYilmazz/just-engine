#include "base.h"

#include "timer.h"

// Timer

Timer new_timer(float32 setup_secs, TimerMode mode) {
    return (Timer) {
        .mode = mode,
        .time_setup = setup_secs,
        .time_elapsed = 0.0,
        .finished = false,
    };
}

void reset_timer(Timer* timer) {
    timer->time_elapsed = 0.0;
    timer->finished = false;
}

void tick_timer(Timer* timer, float32 delta_time) {
    switch (timer->mode) {
    case Timer_NonRepeating:
        if (!timer->finished) {
            timer->time_elapsed += delta_time;
            if (timer->time_elapsed >= timer->time_setup) {
                timer->finished = true;
            }
        }
        break;
    case Timer_Repeating:
        timer->time_elapsed += delta_time;
        if (timer->time_elapsed >= timer->time_setup) {
            timer->time_elapsed = 0.0;
            timer->finished = true;
        }
        break;
    }
}

bool timer_is_finished(Timer* timer) {
    switch (timer->mode) {
    case Timer_NonRepeating:
        return timer->finished;
    case Timer_Repeating:
        if (timer->finished) {
            timer->finished = false;
            return true;
        }
        return false;
    }
}

// SequenceTimer

SequenceTimer new_sequence_timer(float32* checkpoints, uint32 count, TimerMode mode) {
    return (SequenceTimer) {
        .mode = mode,
        .checkpoint_count = count,
        .current_index = 0,
        .checkpoints = checkpoints,
        .time_elapsed = 0.0,
        .pulsed = false,
        .finished = false,
    };
}

// allocates
SequenceTimer new_sequence_timer_evenly_spaced(float32 time_between, uint32 count, TimerMode mode) {
    float32* checkpoints = std_malloc(count * sizeof(float32));
    for (uint32 i = 0; i < count; i++) {
        checkpoints[i] = time_between * (i+1);
    }
    return (SequenceTimer) {
        .mode = mode,
        .checkpoint_count = count,
        .current_index = 0,
        .checkpoints = checkpoints,
        .time_elapsed = 0.0,
        .pulsed = false,
        .finished = false,
    };
}

void reset_sequence_timer(SequenceTimer* timer) {
    timer->current_index = 0;
    timer->time_elapsed = 0.0;
    timer->pulsed = false;
    timer->finished = false;
}

void tick_sequence_timer(SequenceTimer* timer, float delta_time_seconds) {
    if (timer->checkpoint_count == 0) {
        timer->finished = true;
        return;
    }

    switch (timer->mode) {
    case Timer_NonRepeating:
        if (!timer->finished) {
            float checkpoint = timer->checkpoints[timer->current_index];
            timer->time_elapsed += delta_time_seconds;
            if (timer->time_elapsed >= checkpoint) {
                timer->pulsed = true;
                timer->current_index++;
                timer->current_index %= timer->checkpoint_count;
                if (timer->current_index == 0) {
                    timer->finished = true;
                }
            }
        }
        break;
    case Timer_Repeating:
        float checkpoint = timer->checkpoints[timer->current_index];
        timer->time_elapsed += delta_time_seconds;
        if (timer->time_elapsed >= checkpoint) {
            timer->pulsed = true;
            timer->current_index++;
            timer->current_index %= timer->checkpoint_count;
            if (timer->current_index == 0) {
                timer->time_elapsed = 0.0;
            }
        }
        break;
    }
}

bool sequence_timer_has_pulsed(SequenceTimer* timer) {
    if (timer->pulsed) {
        timer->pulsed = false;
        return true;
    }
    return false;
}

bool sequence_timer_is_finished(SequenceTimer* timer) {
    switch (timer->mode) {
    case Timer_NonRepeating:
        return timer->finished;
    case Timer_Repeating:
        if (timer->finished) {
            timer->finished = false;
            return true;
        }
        return false;
    }
}

// StepTime

StepTimer new_step_timer(uint32 count, TimerMode mode) {
    return (StepTimer) {
        .mode = mode,
        .step_count = count,
        .current_step = 0,
        .finished = false,
    };
}

void reset_step_timer(StepTimer* timer) {
    timer->current_step = 0;
    timer->finished = false;
}

void tick_step_timer(StepTimer* timer) {
    switch (timer->mode) {
    case Timer_NonRepeating:
        if (!timer->finished) {
            timer->current_step++;
            if (timer->current_step >= timer->step_count) {
                timer->finished = true;
            }
        }
        break;
    case Timer_Repeating:
        timer->current_step++;
        if (timer->current_step >= timer->step_count) {
            timer->current_step = 0;
            timer->finished = true;
        }
        break;
    }
}

bool step_timer_is_finished(StepTimer* timer) {
    switch (timer->mode) {
    case Timer_NonRepeating:
        return timer->finished;
    case Timer_Repeating:
        if (timer->finished) {
            timer->finished = false;
            return true;
        }
        return false;
    }
}
