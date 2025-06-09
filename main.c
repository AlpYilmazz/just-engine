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

// Example input struct (pack tightly!)
typedef struct {
    uint8 buttons;        // Bitmask: UP, DOWN, LEFT, RIGHT, PUNCH, KICK, etc.
    uint8 stick_x;        // Analog stick (if needed)
    uint8 stick_y;
} PlayerInput;

typedef struct {
    uint32 start_frame;      // Oldest frame in the packet (e.g., current_frame - 2)
    uint8 num_inputs;        // Usually 3 (current + 2 prior frames)
    PlayerInput inputs[3];     // Array of inputs (each 2–4 bytes)
} InputPacket;

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
} ServerMessage; // 14

const usize SERVER_MESSAGE_BYTE_SIZE = 14;
/**
 * @param buffer    should be at least this long: SERVER_MESSAGE_BYTE_SIZE (14)
 */
ServerMessage unpack_server_message(byte* buffer) {
    ServerMessage msg = {0};
    byte* cursor = buffer;

    msg.player_id = *((uint8*)cursor);
    cursor += sizeof(uint8);

    msg.frame_number = just_ntohll(*((uint64*)cursor));
    cursor += sizeof(uint64);

    msg.command.key = just_ntohl(*((int32*)cursor));
    cursor += sizeof(int32);
    
    msg.command.action = *((KeyAction*)cursor);
    cursor += sizeof(KeyAction);

    return msg;
}

// TODO: thread sync? is it necessary for bool
SOCKET g_socket;
bool connect_started = false;
bool is_connected = false;

typedef struct {
    ServerMessage message;
    bool consumed;
} ServerMessageEvent;

__DECLARE__EVENT_SYSTEM__ACCESS_MULTI_THREADED(ServerMessageEvent);
__IMPL_____EVENT_SYSTEM__ACCESS_MULTI_THREADED(ServerMessageEvent);

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
        ServerMessageEvent event = events_iter_consume_next(ServerMessageEvent)(&events_iter);
        dynarray_push_back(RES_frame_id_list, event.message.frame_number);
    }

    LOCAL_server_events_offset = events_iter_end(ServerMessageEvent)(&events_iter);
}

void SYSTEM_FRAME_BOUNDARY_swap_event_buffers_ServerMessageEvent(
    Events(ServerMessageEvent)* RES_server_events
) {
    events_swap_buffers(ServerMessageEvent)(RES_server_events);
}

bool on_read(SOCKET socket, BufferSlice read_buffer, void* arg) {
    FillBuffer* read_fill = arg;

    if (read_buffer.length == 0) {
        free(read_fill->bytes);
        return true; // should_remove
    }
    
    JUST_LOG_INFO("reading received bytes for [ServerMessage: %d]\n", SERVER_MESSAGE_BYTE_SIZE);
    for (usize i = 0; i < read_buffer.length; i++) {
        *read_fill->cursor = read_buffer.bytes[i];
        read_fill->cursor++;
        if (filled_length(read_fill) == SERVER_MESSAGE_BYTE_SIZE) {
            read_fill->cursor = read_fill->bytes;

            ServerMessageEvent event = {
                .message = unpack_server_message(read_fill->bytes),
                .consumed = false,
            };

            JUST_LOG_INFO("sending event ServerMessageEvent\n");
            events_send_single(ServerMessageEvent)(&RES.server_events, event);
        }
    }
    return false;
}

void on_write(SOCKET socket, void* _arg) {
    JUST_LOG_DEBUG("Write Made\n");
}

void on_connect(SOCKET socket, bool success, void* _arg) {
    if (success) {
        JUST_LOG_INFO("Connection Made\n");
        is_connected = true;
    
        FillBuffer* on_read_arg = malloc_fillbuffer(sizeof(ServerMessage));
        network_start_read_stream(socket, on_read, on_read_arg);
    
        // network_write_stream();
    }
    else {
        JUST_LOG_WARN("Connection Failed\n");
    }
}

void server_connect() {
    SOCKET socket = make_socket(SOCKET_TYPE_STREAM);
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
        //         network_write_stream(g_socket, buffer, on_write, NULL);
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