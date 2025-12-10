#pragma once

#include "core.h"
#include "memory/justqueue.h"

typedef enum {
    CORE_STAGE__FRAME_BEGIN = 0,
    //
    CORE_STAGE__INPUT = 1000,
    //
    CORE_STAGE__PREPARE__PRE_PREPARE = 2000,
    CORE_STAGE__PREPARE__PREPARE = 2500,
    CORE_STAGE__PREPARE__POST_PREPARE = 3000,
    //
    CORE_STAGE__UPDATE__PRE_UPDATE = 4000,
    CORE_STAGE__UPDATE__UPDATE = 4500,
    CORE_STAGE__UPDATE__POST_UPDATE = 5000,
    //
    CORE_STAGE__RENDER__QUEUE_RENDER = 6000,
    CORE_STAGE__RENDER__EXTRACT_RENDER = 6500,
    CORE_STAGE__RENDER__RENDER = 7000,
    //
    CORE_STAGE__FRAME_END = __INT32_MAX__,
} JustEngineCoreStage;

typedef void (*SystemFn)(void);

#define MAX_DEP 10

typedef struct {
    usize count;
    SystemFn systems[MAX_DEP];
} SystemDependency;

typedef struct {
    bool run_first;
    bool run_last;
    SystemDependency run_after;
    SystemDependency run_before;
} SystemConstraint;

typedef struct {
    usize count;
    usize capacity;
    SystemFn* edges;
} SystemDAGEdges;

typedef struct {
    SystemFn system;
    usize n_deps;
    SystemDAGEdges edges_from;
    SystemDAGEdges edges_into;
} SystemDAGNode;

typedef struct {
    usize count;
    usize capacity;
    SystemDAGNode* nodes;
    Option(usize) first;
    Option(usize) last;
} SystemDAG;

void system_dag_add_system(SystemDAG* dag, SystemFn system);
void system_dag_add_system_with(SystemDAG* dag, SystemFn system, SystemConstraint constraint);

typedef struct {
    int32 stage_id;
    usize count;
    usize capacity;
    SystemFn* systems;
} AppStage;

AppStage app_stage_from_system_dag(int32 stage_id, SystemDAG* dag);
void app_stage_run_once(AppStage* stage);

typedef struct {
    usize count;
    usize capacity;
    AppStage* stages;
} JustApp;

void just_app_add_stage(JustApp* app, AppStage stage);
void just_app_run_once(JustApp* app);

typedef struct {
    bool end;
    int32 transition_id;
    // --
    int32 chapter_id;
    SystemFn init_system;
    SystemFn deinit_system;
    JustApp app;
} JustChapter;

typedef JustChapter* JustChapterPtr;

typedef struct {
    usize count;
    usize capacity;
    JustChapterPtr* chapters;
} JustChapters;

typedef struct {
    usize count;
    usize capacity;
    int32* stage_ids;
    SystemDAG* stages;
} JustAppBuilder;

void just_app_builder_add_system(JustAppBuilder* app_builder, int32 stage_id, SystemFn system);
void just_app_builder_add_system_with(JustAppBuilder* app_builder, int32 stage_id, SystemFn system, SystemConstraint constraint);
JustApp just_app_builder_build_app(JustAppBuilder* app_builder);

JustAppBuilder* GLOBAL_APP_BUILDER();
void APP_ADD_SYSTEM(int32 stage_id, SystemFn system);
void APP_ADD_SYSTEM_WITH(int32 stage_id, SystemFn system, SystemConstraint constraint);
JustApp BUILD_APP();