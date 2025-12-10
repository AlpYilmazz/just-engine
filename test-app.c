
#include "justengine.h"

uint32 stage_i = 0;
uint32 system_i = 0;

void frame_begin() {
    JUST_LOG_INFO("-- Frame Begin --\n");
    stage_i = 0;
}

void begin_stage() {
    system_i = 0;
    JUST_LOG_INFO("-- begin_stage --\n");
    JUST_LOG_INFO("Stage: %u, System: %u\n", stage_i, system_i);
    system_i++;
}

void system_1() {
    JUST_LOG_INFO("-- system_1 --\n");
    JUST_LOG_INFO("Stage: %u, System: %u\n", stage_i, system_i);
    system_i++;
}

void system_2() {
    JUST_LOG_INFO("-- system_2 --\n");
    JUST_LOG_INFO("Stage: %u, System: %u\n", stage_i, system_i);
    system_i++;
}

void end_stage() {
    JUST_LOG_INFO("-- end_stage --\n");
    JUST_LOG_INFO("Stage: %u, System: %u\n", stage_i, system_i);
    stage_i++;
}


void frame_end() {
    JUST_LOG_INFO("-- Frame End --\n");
}

int main() {
    SET_LOG_LEVEL(LOG_INFO);

    {
        JUST_DEV_MARK();
        APP_ADD_SYSTEM(CORE_STAGE__FRAME_BEGIN, frame_begin);
    }
    {
        JUST_DEV_MARK();
        APP_ADD_SYSTEM(CORE_STAGE__FRAME_END, frame_end);
    }

    #define NSTAGES 3
    for (int32 i = 1; i <= NSTAGES; i++) {
        JUST_DEV_MARK();
        
        APP_ADD_SYSTEM_WITH(i, end_stage, (SystemConstraint) { .run_last = true });

        APP_ADD_SYSTEM_WITH(i, system_2,
            (SystemConstraint) {
                .run_after = {
                    .count = 2,
                    .systems = { system_1, end_stage },
                },
            }
        );

        APP_ADD_SYSTEM_WITH(i, system_1,
            (SystemConstraint) {
                .run_after = {
                    .count = 1,
                    .systems = { end_stage },
                },
                .run_before = {
                    .count = 1,
                    .systems = { begin_stage },
                },
            }
        );

        APP_ADD_SYSTEM_WITH(i, begin_stage, (SystemConstraint) { .run_first = true });
    }

    {
        int32 stage = NSTAGES + 1;
        APP_ADD_SYSTEM(stage, begin_stage);
        APP_ADD_SYSTEM(stage, system_1);
        APP_ADD_SYSTEM(stage, system_2);
        APP_ADD_SYSTEM(stage, end_stage);
    }
    {
        int32 stage = NSTAGES + 2;
        APP_ADD_SYSTEM(stage, begin_stage);
        APP_ADD_SYSTEM(stage, end_stage);
        APP_ADD_SYSTEM(stage, system_2);
        APP_ADD_SYSTEM(stage, system_1);
    }

    JustApp app = BUILD_APP();

    just_app_run_once(&app);
    JUST_DEV_MARK();

    return 0;
}