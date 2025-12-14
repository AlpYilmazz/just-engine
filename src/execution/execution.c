#include "memory/memory.h"

#include "execution.h"

bool system_fn_equals(SystemFn s1, SystemFn s2) {
    return s1.fn == s2.fn;
}

bool system_fn_different(SystemFn s1, SystemFn s2) {
    return s1.fn != s2.fn;
}

// SystemDAG

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
        ASSERT(system_fn_different(s, system));
        dynarray_push_back_custom(edges_from, .edges, s);
    }
    
    SystemDAGEdges edges_into = {0};
    dynarray_reserve_custom(edges_into, .edges, constraint.run_before.count);
    for (usize i = 0; i < constraint.run_before.count; i++) {
        SystemFn s = constraint.run_before.systems[i];
        ASSERT(system_fn_different(s, system));
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

// void system_dag_add_system_void(SystemDAG* dag, SystemFn_Void system) {
//     SystemFn system_fn = {
//         .kind = SYSTEM_FN__VOID,
//         .fn = system,
//     };
//     __system_dag_add_system(dag, system_fn);
// }

// void system_dag_add_system_void_with(SystemDAG* dag, SystemFn_Void system, SystemConstraint constraint) {
//     SystemFn system_fn = {
//         .kind = SYSTEM_FN__VOID,
//         .fn = system,
//     };
//     __system_dag_add_system_with(dag, system_fn, constraint);
// }

// void system_dag_add_system_app_control(SystemDAG* dag, SystemFn_AppControl system) {
//     SystemFn system_fn = {
//         .kind = SYSTEM_FN__APP_CONTROL,
//         .fn = system,
//     };
//     __system_dag_add_system(dag, system_fn);
// }

// void system_dag_add_system_app_control_with(SystemDAG* dag, SystemFn_AppControl system, SystemConstraint constraint) {
//     SystemFn system_fn = {
//         .kind = SYSTEM_FN__APP_CONTROL,
//         .fn = system,
//     };
//     __system_dag_add_system_with(dag, system_fn, constraint);
// }

static bool system_dag_find_system(SystemDAG* dag, SystemFn system, usize* set_index) {
    for (usize i = 0; i < dag->count; i++) {
        if (system_fn_equals(dag->nodes[i].system, system)) {
            if (set_index) *set_index = i;
            return true;
        }
    }
    return false;
}

static bool system_dag_edges_find_edge(SystemDAGEdges edges, SystemFn system, usize* set_index) {
    for (usize i = 0; i < edges.count; i++) {
        if (system_fn_equals(edges.edges[i], system)) {
            if (set_index) *set_index = i;
            return true;
        }
    }
    return false;
}

// AppStage

AppStage app_stage_from_system_dag(int32 stage_id, SystemDAG* dag) {
    // Put 'into' edges to corresponding 'to' edges
    for (usize i = 0; i < dag->count; i++) {
        // 'first' system cannot put 'into' edges
        if (dag->first.is_some && dag->first.value == i) {
            continue;
        }

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

    // Count n_deps
    for (usize i = 0; i < dag->count; i++) {
        // 'last' system cannot create deps
        if (dag->last.is_some && dag->last.value == i) {
            continue;
        }

        SystemDAGNode* this_node = &dag->nodes[i];
        SystemFn this_system = this_node->system;

        for (usize edge_i = 0; edge_i < this_node->edges_into.count; edge_i++) {
            SystemFn into_system = this_node->edges_into.edges[edge_i];
            usize index;
            if (system_dag_find_system(dag, into_system, &index)) {
                SystemDAGNode* into_node = &dag->nodes[index];
                into_node->n_deps++;
            }
        }
    }

    Queue(usize) q = queue_new(usize)(dag->count);

    if (dag->first.is_some) {
        dag->nodes[dag->first.value].n_deps = 0;
        queue_push(usize)(&q, dag->first.value);
    }

    for (usize i = 0; i < dag->count; i++) {
        if (dag->first.is_some && dag->first.value == i) {
            continue;
        }
        if (dag->nodes[i].n_deps == 0) {
            queue_push(usize)(&q, i);
        }
    }

    AppStage stage = {0};
    stage.stage_id = stage_id;
    dynarray_reserve_custom(stage, .systems, dag->count);

    usize popped_index;
    while (queue_pop(usize)(&q, &popped_index)) {
        if (dag->last.is_some && dag->last.value == popped_index) {
            continue;
        }
        
        SystemDAGNode* this_node = &dag->nodes[popped_index];

        dynarray_push_back_custom(stage, .systems, this_node->system);
        
        // Remove deps
        for (usize edge_i = 0; edge_i < this_node->edges_into.count; edge_i++) {
            SystemFn into_system = this_node->edges_into.edges[edge_i];
            usize into_node_index;
            if (system_dag_find_system(dag, into_system, &into_node_index)) {
                SystemDAGNode* into_node = &dag->nodes[into_node_index];
                into_node->n_deps--;
                if (into_node->n_deps == 0) {
                    queue_push(usize)(&q, into_node_index);
                }
            }
        }
    }

    if (dag->last.is_some) {
        SystemFn last_system = dag->nodes[dag->last.value].system;
        dynarray_push_back_custom(stage, .systems, last_system);
    }

    // Circular
    ASSERT(stage.count == dag->count);

    dynarray_free_custom(*dag, .nodes);
    return stage;
}

void app_control_reset(AppControl* app_control) {
    app_control->return_now = false;
}

void app_stage_run_once(AppStage* stage, AppControl* app_control) {
    for (usize i = 0; i < stage->count; i++) {
        app_control_reset(app_control);
        SystemFn system = stage->systems[i];
        if (system.fn != NULL) {
            switch (system.kind) {
            case SYSTEM_FN__VOID:
                system.fn_void();
                break;
            case SYSTEM_FN__APP_CONTROL:
                system.fn_app_control(app_control);
                if (app_control->return_now) {
                    return;
                }
                break;
            default:
                UNREACHABLE();
            }
        }
    }
}

// JustApp

void just_app_add_stage(JustApp* app, AppStage stage) {
    usize i_insert = 0;
    for (i_insert = 0; i_insert < app->count; i_insert++) {
        ASSERT(stage.stage_id != app->stages[i_insert].stage_id);
        if (stage.stage_id < app->stages[i_insert].stage_id) {
            break;
        }
    }
    dynarray_insert_custom(*app, .stages, i_insert, stage);
}

void just_app_run_once(JustApp* app) {
    for (usize i = 0; i < app->count; i++) {
        AppControl app_control = {0};
        app_stage_run_once(&app->stages[i], &app_control);
        if (app_control.return_now) {
            return;
        }
    }
}

// JustAppBuilder

static usize just_app_builder_find_or_create_place(JustAppBuilder* app_builder, int32 stage_id) {
    usize add_index = app_builder->count;
    for (usize i = 0; i < app_builder->count; i++) {
        if (stage_id == app_builder->stage_ids[i]) {
            add_index = i;
            break;
        }
    }
    if (add_index == app_builder->count) {
        dynarray_push_back_custom_2(*app_builder, .stage_ids, stage_id, .stages, ((SystemDAG){0}));
    }
    return add_index;
}

void just_app_builder_add_system(JustAppBuilder* app_builder, int32 stage_id, SystemFn system) {
    usize add_index = just_app_builder_find_or_create_place(app_builder, stage_id);
    system_dag_add_system(&app_builder->stages[add_index], system);
}

void just_app_builder_add_system_with(JustAppBuilder* app_builder, int32 stage_id, SystemFn system, SystemConstraint constraint) {
    usize add_index = just_app_builder_find_or_create_place(app_builder, stage_id);
    system_dag_add_system_with(&app_builder->stages[add_index], system, constraint);
}

// void just_app_builder_add_system_void(JustAppBuilder* app_builder, int32 stage_id, SystemFn_Void system) {
//     usize add_index = just_app_builder_find_or_create_place(app_builder, stage_id);
//     system_dag_add_system(&app_builder->stages[add_index], system);
// }

// void just_app_builder_add_system_void_with(JustAppBuilder* app_builder, int32 stage_id, SystemFn_Void system, SystemConstraint constraint) {
//     usize add_index = just_app_builder_find_or_create_place(app_builder, stage_id);
//     system_dag_add_system_with(&app_builder->stages[add_index], system, constraint);
// }

// void just_app_builder_add_system_app_control(JustAppBuilder* app_builder, int32 stage_id, SystemFn_AppControl system) {
//     usize add_index = just_app_builder_find_or_create_place(app_builder, stage_id);
//     system_dag_add_system(&app_builder->stages[add_index], system);
// }

// void just_app_builder_add_system_app_control_with(JustAppBuilder* app_builder, int32 stage_id, SystemFn_AppControl system, SystemConstraint constraint) {
//     usize add_index = just_app_builder_find_or_create_place(app_builder, stage_id);
//     system_dag_add_system_with(&app_builder->stages[add_index], system, constraint);
// }

JustApp just_app_builder_build_app(JustAppBuilder* app_builder) {
    JustApp app = {0};
    for (usize i = 0; i < app_builder->count; i++) {
        int32 stage_id = app_builder->stage_ids[i];
        SystemDAG* dag = &app_builder->stages[i];
        AppStage app_stage = app_stage_from_system_dag(stage_id, dag);
        just_app_add_stage(&app, app_stage);
    }
    dynarray_free_custom_2(*app_builder, .stage_ids, .stages);
    return app;
}

// -----

static JustAppBuilder JUST_APP_BUILDER = {0};

JustAppBuilder* GLOBAL_APP_BUILDER() {
    return &JUST_APP_BUILDER;
}

void APP_ADD_SYSTEM(int32 stage_id, SystemFn system) {
    just_app_builder_add_system(&JUST_APP_BUILDER, stage_id, system);
}

void APP_ADD_SYSTEM_WITH(int32 stage_id, SystemFn system, SystemConstraint constraint) {
    just_app_builder_add_system_with(&JUST_APP_BUILDER, stage_id, system, constraint);
}

JustApp BUILD_APP() {
    return just_app_builder_build_app(&JUST_APP_BUILDER);
}