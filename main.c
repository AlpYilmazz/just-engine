
// typedef     unsigned char           uint8;
// typedef     unsigned short          uint16;
// typedef     unsigned int            uint32;
// typedef     unsigned long long      uint64;

// typedef     char                    int8;
// typedef     short                   int16;
// typedef     int                     int32;
// typedef     long long               int64;

// typedef     float                   float32;
// typedef     double                  float64;

// typedef     uint8                   bool;


// #define DECL_EVENT_SYSTEM_BASE(TEvent, t_event)                                                                          \
//     typedef struct {                                                                                                \
//         uint32 count;                                                                                               \
//         uint32 capacity;                                                                                            \
//         TEvent* items;                                                                                              \
//     } EventBuffer_##TEvent;                                                                                       \
//                                                                                                                     \
//     void event_buffer_##t_event##_push_back(EventBuffer_##TEvent* buffer, TEvent item);                           \
//     void event_buffer_##t_event##_push_back_batch(EventBuffer_##TEvent* buffer, TEvent* items, uint32 count);     \
//     void event_buffer_##t_event##_clear(EventBuffer_##TEvent* buffer);                                            \
//                                                                                                                     \
//     typedef struct {                                                                                                \
//         EventBuffer_##TEvent event_buffers[2];                                                                    \
//         uint8 this_frame_ind;                                                                                       \
//     } Events_##TEvent;                                                                                            \
//                                                                                                                     \
//     uint32 just_engine_events_##t_event##_this_frame_buffer_count(Events_##TEvent* events);                           \
//     void just_engine_events_##t_event##_send_single(Events_##TEvent* events, TEvent event);                           \
//     void just_engine_events_##t_event##_send_batch(Events_##TEvent* events, TEvent* event_list, uint32 count);        \
//     void just_engine_events_##t_event##_swap_buffers(Events_##TEvent* events);                                        \
//                                                                                                                     \
//     typedef struct {                                                                                                \
//         uint32 index;                                                                                               \
//         Events_##TEvent* events;                                                                                  \
//     } EventsIter_##TEvent;                                                                                        \
//                                                                                                                     \
//     EventsIter_##TEvent just_engine_events_iter_##t_event##_new(Events_##TEvent* events, uint32 offset);        \
//     EventsIter_##TEvent just_engine_events_iter_##t_event##_new_zero(Events_##TEvent* events);                  \
//     bool just_engine_events_iter_##t_event##_has_next(EventsIter_##TEvent* iter);                                 \
//     TEvent just_engine_events_iter_##t_event##_read_next(EventsIter_##TEvent* iter);                              \

// #define DECL_EVENT_SYSTEM_EXT_CONSUMABLE(TEvent, t_event)                                                       \
//     TEvent just_engine_events_iter_##t_event##_consume_next(EventsIter_##TEvent* iter);                           


// typedef struct {
//     uint32 id;
//     bool consumed;
// } TestEvent;

// DECL_EVENT_SYSTEM_BASE(TestEvent, test_event)
// DECL_EVENT_SYSTEM_EXT_CONSUMABLE(TestEvent, test_event)

#include <stdio.h>

#include "justengine.h"

void test_rayhit() {
    Ray2 ray = {
        .position = {0, 0},
        .direction = {1, 0},
    };

    #define CIRCLE_TEST_CASES 9
    CircleCollider circle_cases[CIRCLE_TEST_CASES] = {
        {
            .center = {-5, 1},
            .radius = 2,
        },
        {
            .center = {-5, 1},
            .radius = 7,
        },
        {
            .center = {-5, 1},
            .radius = 100,
        },

        {
            .center = {5, 3},
            .radius = 2,
        },
        {
            .center = {5, 3},
            .radius = 4,
        },
        {
            .center = {5, 3},
            .radius = 100,
        },

        {
            .center = {15, 1},
            .radius = 2,
        },
        {
            .center = {15, 1},
            .radius = 7,
        },
        {
            .center = {15, 1},
            .radius = 100,
        },
    };

    for (int i = 0; i < CIRCLE_TEST_CASES; i++) {
        CircleCollider circle = circle_cases[i];
        bool rayhit = just_engine_check_rayhit_circle(ray, circle, 10);
        printf("%d - Circle: {%d, %d} (%d)    ->    Rayhit: %s\n"
            , i, (int)circle.center.x, (int)circle.center.y, (int)circle.radius, rayhit ? "true" : "false");
    }
}

typedef enum {
    HEADER_IDENTITY = 1,
    HEADER_POSITION_UPDATE,
} HeaderEnum;

typedef struct {
    char name[10];
} PlayerIdentity;

typedef enum {
    KEY_ACTION_PRESS = 0,
    KEY_ACTION_RELEASE,
} KeyAction;

typedef struct {
    int32 key; // 4
    KeyAction action; // 1
} PlayerCommand; // 5

typedef struct {
    uint8 player_id; // 1
    uint64 frame_number; // 8
    PlayerCommand command; // 5
} ServerMessage;

// TODO: thread sync? is it necessary for bool
SOCKET g_socket;
bool connect_started = false;
bool is_connected = false;

typedef struct {
    ServerMessage message;
    bool consumed;
} ServerMessageEvent;

// _DECLARE__EVENT_SYSTEM__ACCESS_MULTI_THREADED(ServerMessageEvent);
// _IMPL_____EVENT_SYSTEM__ACCESS_MULTI_THREADED
DECLARE__EVENT_SYSTEM__ACCESS_MULTI_THREADED(ServerMessageEvent);
DEFINE_IMPL__EVENT_SYSTEM__ACCESS_MULTI_THREADED(ServerMessageEvent);

typedef struct {
    usize count;
    usize capacity;
    uint64* items;
} FrameIdList;

struct {
    Events(ServerMessageEvent) server_events;
    FrameIdList frame_id_list;
} RES = {
    .server_events = LAZY_INIT,
    .frame_id_list = STRUCT_ZERO_INIT,
};

void SYSTEM_POST_UPDATE_read_server_messages(
    Events(ServerMessageEvent)* RES_server_events,
    FrameIdList* RES_frame_id_list
) {
    static uint32 LOCAL_server_events_offset = 0;

    EventsIter(ServerMessageEvent) events_iter = events_begin_iter(ServerMessageEvent)(RES_server_events, LOCAL_server_events_offset);

    while (events_iter_has_next(ServerMessageEvent)(&events_iter)) {
        JUST_LOG_INFO("next event\n");
        ServerMessageEvent event = events_iter_read_next(ServerMessageEvent)(&events_iter);
        dynarray_push_back(RES_frame_id_list, event.message.frame_number);

        // FrameIdList* arr = RES_frame_id_list;
        // uint64 item = event.message.frame_number;
        // do {
        //     const uint32 DYNARRAY_INITIAL_CAPACITY = 2;
        //     const uint32 DYNARRAY_GROWTH_FACTOR = 2;

        //     if ((arr)->capacity == 0) {
        //         (arr)->capacity = DYNARRAY_INITIAL_CAPACITY;
        //         (arr)->items = malloc((arr)->capacity * sizeof((item)));
        //     }
        //     else if ((arr)->count == (arr)->capacity) {
        //         (arr)->capacity = DYNARRAY_GROWTH_FACTOR * (arr)->capacity;
        //         (arr)->items = realloc((arr)->items, (arr)->capacity * sizeof((item)));
        //     }

        //     (arr)->items[(arr)->count] = (item);
        //     (arr)->count++;
        // } while(0);
    }

    LOCAL_server_events_offset = events_iter_end(ServerMessageEvent)(&events_iter);
}

void SYSTEM_FRAME_BOUNDARY_swap_event_buffers_ServerMessageEvent(
    Events(ServerMessageEvent)* RES_server_events
) {
    events_swap_buffers(ServerMessageEvent)(RES_server_events);
}

bool on_read(SOCKET socket, BufferSlice read_buffer, void* arg) {
    JUST_LOG_DEBUG("on_read\n");
    JUST_LOG_DEBUG("---- read_buffer.bytes: %p\n", read_buffer.bytes);
    JUST_LOG_DEBUG("---- read_buffer.length: %d\n", read_buffer.length);

    FillBuffer* read_fill = arg;

    if (read_buffer.length == 0) {
        free(read_fill->bytes);
        return true; // should_remove
    }
    
    ServerMessage msg = {0};
    usize msg_byte_size = 14;
    JUST_LOG_DEBUG("reading ServerMessage: %d\n", msg_byte_size);
    for (usize i = 0; i < read_buffer.length; i++) {
        *read_fill->cursor = read_buffer.bytes[i];
        read_fill->cursor++;
        if (filled_length(read_fill) == msg_byte_size) {
            read_fill->cursor = read_fill->bytes;
            byte* cursor = read_fill->bytes;

            msg.player_id = *((uint8*)cursor);
            cursor += sizeof(uint8);

            msg.frame_number = just_ntohll(*((uint64*)cursor));
            cursor += sizeof(uint64);

            msg.command.key = just_ntohl(*((int32*)cursor));
            cursor += sizeof(int32);
            
            msg.command.action = *((KeyAction*)cursor);
            cursor += sizeof(KeyAction);

            // TODO: event ServerMessage
            ServerMessageEvent event = {
                .message = msg,
                .consumed = false,
            };
            JUST_LOG_INFO("sending event ServerMessage\n");
            events_send_single(ServerMessageEvent)(&RES.server_events, event);
            // ServerMessageEvent__events_send_single(&RES.server_events, event);
        }
    }
    return false;
}

void on_write(SOCKET socket, void* _arg) {
    JUST_LOG_DEBUG("Write Made\n");
}

void on_connect(SOCKET socket, void* _arg) {
    JUST_LOG_DEBUG("Connection Made\n");
    is_connected = true;

    FillBuffer* on_read_arg = malloc_fillbuffer(sizeof(ServerMessage));
    JUST_LOG_DEBUG("---- on_read_arg: %p\n", on_read_arg);
    JUST_LOG_DEBUG("---- sizeof(on_read_arg): %d\n", sizeof(*on_read_arg));
    JUST_LOG_DEBUG("---- on_read_arg->length: %d\n", on_read_arg->length);
    JUST_LOG_DEBUG("---- on_read_arg->cursor: %p\n", on_read_arg->cursor);
    JUST_LOG_DEBUG("---- on_read_arg->bytes: %p\n", on_read_arg->bytes);
    on_read_arg->cursor[0] = 10;
    JUST_LOG_DEBUG("-c- 1\n");
    on_read_arg->cursor[2] = 12;
    JUST_LOG_DEBUG("-c- 2\n");
    JUST_LOG_DEBUG("---- on_read_arg->cursor[0]: %x\n", on_read_arg->cursor[0]);
    JUST_LOG_DEBUG("---- on_read_arg->cursor[2]: %x\n", on_read_arg->cursor[2]);
    network_start_read(socket, on_read, on_read_arg);

    // network_write_buffer();
}

void server_connect() {
    SOCKET socket = make_socket(SOCKET_TYPE_TCP);
    SocketAddr addr = {
        .host = "127.0.0.1",
        .port = 8080,
    };
    network_connect(socket, addr, on_connect, NULL);
    g_socket = socket;
}

int main() {
    SET_LOG_LEVEL(LOG_LEVEL_INFO);

    InitWindow(1000, 1000, "Test");
    SetTargetFPS(60);

    init_network_thread();

    {
        RES.server_events = events_create(ServerMessageEvent)();
    }

    usize ping_msg_len = 7;
    byte ping_msg[] = {0x25, 0x25, 0x04, 'p', 'i', 'n', 'g'};
    Timer timer = new_timer(5, Timer_Repeating);
    
    while (!WindowShouldClose()) {
        float32 delta_time = GetFrameTime();
        
        if (!connect_started && !is_connected && IsKeyPressed(KEY_SPACE)) {
            connect_started = true;
            server_connect();
        }

        // if (is_connected) {
        //     tick_timer(&timer, delta_time);
        //     if (timer_is_finished(&timer)) {
        //         JUST_LOG_DEBUG("Timer finished\n");
        //         BufferSlice buffer = {
        //             .bytes = ping_msg,
        //             .length = ping_msg_len,
        //         };
        //         network_write_buffer(g_socket, buffer, on_write, NULL);
        //     }
        // }

        SYSTEM_POST_UPDATE_read_server_messages(
            &RES.server_events,
            &RES.frame_id_list
        );

        SYSTEM_FRAME_BOUNDARY_swap_event_buffers_ServerMessageEvent(
            &RES.server_events
        );

        Color bg_color = connect_started ? 
            is_connected ? GREEN : YELLOW
            : RED;

        BeginDrawing();
        ClearBackground(bg_color);
            
            for (usize i = 0; i < RES.frame_id_list.count; i++) {
                uint64 frame_id = RES.frame_id_list.items[i];
                const char* frame_id_text = TextFormat("%llu", frame_id);
                DrawText(frame_id_text, 100, 50 + (50 * i), 30, BLACK);
            }

        EndDrawing();
    }
}

int _main() {

    InitWindow(1000, 1000, "Test");
    SetTargetFPS(60);

    Camera2D camera = {0};
    camera.zoom = 1.0;
    camera.offset = (Vector2) {
        .x = 1000 / 2.0,
        .y = 1000 / 2.0,
    };

    Texture fire_texture = LoadTexture("test-assets\\fire.png");
    Vector2 position = { 300, 300 };
    Vector2 size = { 490 * 0.1, 970 * 0.1 };
    Vector2 origin = {
        .x = size.x * 0.5, // size.x * 0.5,
        .y = size.y, // size.y * 0.5,
    };
    float32 rotation = 0.0;

    while (!WindowShouldClose()) {

        float32 delta_time = GetFrameTime();

        if (!IsKeyDown(KEY_SPACE)) {
            rotation += 20.0 * delta_time;
            rotation -= (rotation > 360.0) ? 360.0 : 0.0;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            rotation = 0.0;
        }

        BeginDrawing();
        ClearBackground(WHITE);
            BeginMode2D(camera);

                DrawTexturePro(
                    fire_texture,
                    (Rectangle) {0, 0, fire_texture.width, fire_texture.height},
                    (Rectangle) {
                        .x = position.x,
                        .y = position.y,
                        .width = size.x,
                        .height = size.y,
                    },
                    origin,
                    rotation,
                    WHITE
                );

                DrawTexturePro(
                    fire_texture,
                    (Rectangle) {0, 0, -fire_texture.width, fire_texture.height},
                    (Rectangle) {
                        .x = -position.x,
                        .y = position.y,
                        .width = size.x,
                        .height = size.y,
                    },
                    (Vector2) {0, 0},
                    0,
                    WHITE
                );

                DrawTexturePro(
                    fire_texture,
                    (Rectangle) {0, 0, fire_texture.width, -fire_texture.height},
                    (Rectangle) {
                        .x = position.x,
                        .y = -position.y,
                        .width = size.x,
                        .height = size.y,
                    },
                    (Vector2) {0, 0},
                    0,
                    WHITE
                );

                DrawTexturePro(
                    fire_texture,
                    (Rectangle) {0, 0, -fire_texture.width, -fire_texture.height},
                    (Rectangle) {
                        .x = -position.x,
                        .y = -position.y,
                        .width = size.x,
                        .height = size.y,
                    },
                    (Vector2) {0, 0},
                    0,
                    WHITE
                );

                DrawCircleV(position, 10, LIME); // tex origin
                DrawCircle(0, 0, 10, RED); // origin

            EndMode2D();
        EndDrawing();
    }

    return 0;
}