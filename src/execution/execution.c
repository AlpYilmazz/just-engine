#include "memory/memory.h"

#include "execution.h"



void stage_executor_perform_once(StageExecutor* stage_executor) {
    for (usize i = 0; i < stage_executor->count; i++) {
        stage_executor->systems[i]();
    }
}

Executor new_executor() {
    return (Executor) {0};
}

void executor_add_stage(Executor* executor, StageExecutor stage_executor) {
    usize i_insert = 0;
    for (i_insert = 0; i_insert < executor->count; i_insert++) {
        ASSERT(stage_executor.stage != executor->stages[i_insert].stage);
        if (stage_executor.stage < executor->stages[i_insert].stage) {
            break;
        }
    }
    dynarray_insert_custom(*executor, .stages, i_insert, stage_executor);
}

void executor_perform_once(Executor* executor) {
    for (usize i = 0; i < executor->count; i++) {
        stage_executor_perform_once(&executor->stages[i]);
    }
}