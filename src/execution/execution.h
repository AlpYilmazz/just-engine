#pragma once

#include "core.h"

typedef enum {
    STAGE__FRAME_BEGIN = 0,
    //
    STAGE__INPUT = 1000,
    //
    STAGE__PREPARE__PRE_PREPARE = 2000,
    STAGE__PREPARE__PREPARE = 2500,
    STAGE__PREPARE__POST_PREPARE = 3000,
    //
    STAGE__UPDATE__PRE_UPDATE = 4000,
    STAGE__UPDATE__UPDATE = 4500,
    STAGE__UPDATE__POST_UPDATE = 5000,
    //
    STAGE__RENDER__QUEUE_RENDER = 6000,
    STAGE__RENDER__EXTRACT_RENDER = 6500,
    STAGE__RENDER__RENDER = 7000,
    //
    STAGE__FRAME_END = __INT32_MAX__,
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

void system_dag_add_system(SystemDAG* dag, SystemFn system) {
    SystemDAGNode node = {
        .system = system,
    };
    dynarray_push_back_custom(*dag, .nodes, node);
}

void system_dag_add_system_with(SystemDAG* dag, SystemFn system, SystemConstraint constraint) {
    ASSERT(!(dag->first.is_some && constraint.run_first));
    ASSERT(!(dag->last.is_some && constraint.run_last));
    ASSERT(!(constraint.run_first && constraint.run_last));

    SystemDAGEdges edges_from = {0};
    dynarray_reserve_custom(edges_from, .edges, constraint.run_after.count);
    for (usize i = 0; i < constraint.run_after.count; i++) {
        SystemFn s = constraint.run_after.systems[i];
        ASSERT(s != system);
        dynarray_push_back_custom(edges_from, .edges, s);
    }
    
    SystemDAGEdges edges_into = {0};
    dynarray_reserve_custom(edges_into, .edges, constraint.run_before.count);
    for (usize i = 0; i < constraint.run_before.count; i++) {
        SystemFn s = constraint.run_before.systems[i];
        ASSERT(s != system);
        dynarray_push_back_custom(edges_into, .edges, s);
    }

    SystemDAGNode node = {
        .system = system,
        .edges_from = edges_from,
        .edges_into = edges_into,
    };
    usize index = dag->count;
    dynarray_push_back_custom(*dag, .nodes, node);

    if (constraint.run_first) {
        dag->first = (Option(usize)) Option_Some(index);
    }
    if (constraint.run_last) {
        dag->last = (Option(usize)) Option_Some(index);
    }
}

bool system_dag_find_system(SystemDAG* dag, SystemFn system, usize* set_index) {
    for (usize i = 0; i < dag->count; i++) {
        if (dag->nodes[i].system == system) {
            set_index = i;
            return true;
        }
    }
    return false;
}

bool system_dag_edges_find_edge(SystemDAGEdges edges, SystemFn system, usize* set_index) {
    for (usize i = 0; i < edges.count; i++) {
        if (edges.edges[i] == system) {
            if (set_index) set_index = i;
            return true;
        }
    }
    return false;
}

typedef struct {
    int32 stage;
    usize count;
    usize capacity;
    SystemFn* systems;
} StageExecutor;

StageExecutor stage_executor_from_system_dag(SystemDAG* dag) {
    for (usize i = 0; i < dag->count; i++) {
        SystemDAGNode* this_node = &dag->nodes[i];
        SystemFn this_system = this_node->system;

        for (usize edge_i = 0; edge_i < this_node->edges_from.count; edge_i++) {
            SystemFn from_system = this_node->edges_from.edges[edge_i];
            usize index;
            if (system_dag_find_system(dag, from_system, &index)) {
                SystemDAGNode* from_node = &dag->nodes[index];
                if (!system_dag_edges_find_edge(from_node->edges_into, this_system, NULL)) {
                    dynarray_push_back_custom(from_node->edges_into, .edges, this_system);
                } 
            }
        }
    }

    for (usize i = 0; i < dag->count; i++) {
        SystemDAGNode* this_node = &dag->nodes[i];
        SystemFn this_system = this_node->system;

        for (usize edge_i = 0; edge_i < this_node->edges_from.count; edge_i++) {
            SystemFn into_system = this_node->edges_from.edges[edge_i];
            usize index;
        }
    }
}

typedef struct {
    usize count;
    usize capacity;
    StageExecutor* stages;
} Executor;