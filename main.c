#include <stdio.h>
#include <assert.h>
#include <stdatomic.h>

#include "justengine.h"

// typedef enum {
//     GAMEPLAY_BUTTON_INPUT_UP = 0,
//     GAMEPLAY_BUTTON_INPUT_RIGHT,
//     GAMEPLAY_BUTTON_INPUT_DOWN,
//     GAMEPLAY_BUTTON_INPUT_LEFT,
//     GAMEPLAY_BUTTON_INPUT_ATTACK,
//     GAMEPLAY_BUTTON_INPUT_BLOCK,
// } GameplayButtonInputs;

// typedef enum {
//     GAMEPLAY_AXIS_INPUT_MOVE_X = 0,
//     GAMEPLAY_AXIS_INPUT_MOVE_Y,
// } GameplayAxisInputs;

// typedef struct {
//     uint8 buttons;     // 1     // Bitmask: UP, RIGHT, DOWN, LEFT, ATTACK, BLOCK
//     int8 move_stick_x; // 1     // Analog stick
//     int8 move_stick_y; // 1
// } PlayerInput; // 3

// typedef struct {
//     uint32 this_frame;     // 4   // Current frame in the packet // uint16 -> ~18 min (60 fps)
//     uint8 num_inputs;      // 1   // Usually 3 (current + 2 prior frames)
//     PlayerInput inputs[3]; // 9   // Array of inputs, current -> inputs[0]
// } InputPacket; // 14

// const usize INPUT_PACKET_BYTE_SIZE = 14;

// /**
//  * @param buffer    should be at least this long: INPUT_PACKET_BYTE_SIZE (14)
//  */
// void pack_input_packet(byte* buffer, InputPacket packet) {
//     byte* cursor = buffer;

//     *((int32*)cursor) = just_htonl(packet.this_frame);
//     cursor += sizeof(int32);
    
//     *((uint8*)cursor) = packet.num_inputs;
//     cursor += sizeof(uint8);
    
//     for (uint8 i = 0; i < 3; i++) {
//         PlayerInput player_input = packet.inputs[i];

//         *((uint8*)cursor) = player_input.buttons;
//         cursor += sizeof(uint8);
        
//         *((uint8*)cursor) = player_input.move_stick_x;
//         cursor += sizeof(uint8);
        
//         *((uint8*)cursor) = player_input.move_stick_y;
//         cursor += sizeof(uint8);
//     }
// }

// /**
//  * @param buffer    should be at least this long: INPUT_PACKET_BYTE_SIZE (14)
//  */
// InputPacket unpack_input_packet(byte* buffer) {
//     InputPacket packet = {0};
//     byte* cursor = buffer;

//     packet.this_frame = just_ntohl(*((int32*)cursor));
//     cursor += sizeof(int32);
    
//     packet.num_inputs = *((uint8*)cursor);
//     cursor += sizeof(uint8);
    
//     PlayerInput player_input = {0};
//     for (uint8 i = 0; i < 3; i++) {
//         player_input.buttons = *((uint8*)cursor);
//         cursor += sizeof(uint8);
        
//         player_input.move_stick_x = *((uint8*)cursor);
//         cursor += sizeof(uint8);
        
//         player_input.move_stick_y = *((uint8*)cursor);
//         cursor += sizeof(uint8);

//         packet.inputs[i] = player_input;
//     }

//     return packet;
// }

// // TODO: thread sync? is it necessary for bool
// Socket g_socket;
// bool connect_started = false;
// bool is_connected = false;
// bool connection_handled = false;
// bool just_connected = false;

// typedef struct {
//     InputPacket packet;
//     bool consumed;
// } InputPacketEvent;

// __DECLARE__EVENT_SYSTEM__ACCESS_MULTI_THREADED(InputPacketEvent);
// __IMPL_____EVENT_SYSTEM__ACCESS_MULTI_THREADED(InputPacketEvent);

// #define MAX_ROLLBACK_FRAMES 100
// typedef struct {
//     uint32 current_frame;
//     float32 delta_time;
//     bool is_real;

//     PlayerInput other_player_frame_input;

//     SpriteEntityId this_player;
//     GamepadInputs this_player_inputs;
    
//     SpriteEntityId other_player;
//     GamepadInputs other_player_inputs;
// } GameState;

// struct {
//     atomic_uint current_frame;

//     TextureAssets texture_assets;
//     SpriteCameraStore camera_store;
//     SpriteStore sprite_store;

//     InputPacket input_packet;
//     Events(InputPacketEvent) input_packet_events;

//     GameState __game_state;
//     uint32 saved_frames_count;
//     GameState saved_game_states[MAX_ROLLBACK_FRAMES];

//     // SpriteEntityId this_player;
//     // SpriteEntityId other_player;
// } RES = {
//     .current_frame = ATOMIC_VAR_INIT(0),

//     .texture_assets = LAZY_INIT,
//     .camera_store = STRUCT_ZERO_INIT,
//     .sprite_store = STRUCT_ZERO_INIT,

//     .input_packet = STRUCT_ZERO_INIT,
//     .input_packet_events = LAZY_INIT,

//     .__game_state = STRUCT_ZERO_INIT,
//     .saved_frames_count = 0,
//     .saved_game_states = STRUCT_ZERO_INIT,

//     // .this_player = STRUCT_ZERO_INIT,
//     // .other_player = STRUCT_ZERO_INIT,
// };

// struct {
//     PreparedRenderSprites prepared_render_sprites;
// } RENDER_RES = {
//     .prepared_render_sprites = STRUCT_ZERO_INIT,
// };

// void handle_player_input(GamepadInputs* gamepad_inputs, PlayerInput input) {
//     update_gamepad_button_state(gamepad_inputs, GAMEPLAY_BUTTON_INPUT_UP, input.buttons & (1 << 0));
//     update_gamepad_button_state(gamepad_inputs, GAMEPLAY_BUTTON_INPUT_RIGHT, input.buttons & (1 << 1));
//     update_gamepad_button_state(gamepad_inputs, GAMEPLAY_BUTTON_INPUT_DOWN, input.buttons & (1 << 2));
//     update_gamepad_button_state(gamepad_inputs, GAMEPLAY_BUTTON_INPUT_LEFT, input.buttons & (1 << 3));
//     update_gamepad_button_state(gamepad_inputs, GAMEPLAY_BUTTON_INPUT_ATTACK, input.buttons & (1 << 4));
//     update_gamepad_button_state(gamepad_inputs, GAMEPLAY_BUTTON_INPUT_BLOCK, input.buttons & (1 << 5));
//     update_gamepad_axis_value(gamepad_inputs, GAMEPLAY_AXIS_INPUT_MOVE_X, ((float32)input.move_stick_x)/127.0f);
//     update_gamepad_axis_value(gamepad_inputs, GAMEPLAY_AXIS_INPUT_MOVE_Y, ((float32)input.move_stick_y)/127.0f);
// }

// void clone_sprite_from_into(SpriteStore* sprite_store, SpriteEntityId dst_id, SpriteEntityId src_id) {
//     SpriteTransform src_transform = RES.sprite_store.transforms[src_id.id];
//     Sprite src_sprite = RES.sprite_store.sprites[src_id.id];
    
//     SpriteTransform* dst_transform = &RES.sprite_store.transforms[dst_id.id];
//     Sprite* dst_sprite = &RES.sprite_store.sprites[dst_id.id];

//     *dst_transform = src_transform;
//     *dst_sprite = src_sprite;
// }

// void set_network_input_for_game_state(GameState* game_state, PlayerInput input) {
//     game_state->is_real = true;
//     game_state->other_player_frame_input = input;
// }

// void reset_game_state(GameState* this_game_state, GameState* prev_game_state) {
//     this_game_state->other_player_inputs = this_game_state->other_player_inputs;
//     if (!this_game_state->is_real) {
//         this_game_state->other_player_frame_input = prev_game_state->other_player_frame_input;
//     }
//     handle_player_input(&this_game_state->other_player_inputs, this_game_state->other_player_frame_input);
    
//     clone_sprite_from_into(&RES.sprite_store, this_game_state->this_player, prev_game_state->this_player);
//     clone_sprite_from_into(&RES.sprite_store, this_game_state->other_player, prev_game_state->other_player);
// }

// Option(uint32) SYSTEM_INPUT_read_input_packets(
//     GameState* RES_game_state,
//     GameState* RES_saved_game_states,
//     Events(InputPacketEvent)* RES_input_packet_events
// ) {
//     static uint32 LOCAL_server_events_offset = 0;

//     uint32 current_frame = RES_game_state->current_frame;

//     bool any_simulation = false;
//     uint32 simulate_starting_from_frame = _UI32_MAX;

//     EventsIter(InputPacketEvent) events_iter = events_begin_iter(InputPacketEvent)(RES_input_packet_events, LOCAL_server_events_offset);

//     while (events_iter_has_next(InputPacketEvent)(&events_iter)) {
//         InputPacketEvent event = events_iter_consume_next(InputPacketEvent)(&events_iter);
//         InputPacket input_packet = event.packet;
//         JUST_LOG_INFO("packet: %d\n", input_packet.this_frame);

//         // (input.current - 2) -> (input.current - 1) -> (input.current)
//         for (int32 input_frame_i = input_packet.num_inputs - 1; input_frame_i >= 0; input_frame_i--) {
//             uint32 input_frame = input_packet.this_frame - input_frame_i;
//             PlayerInput input = input_packet.inputs[input_frame_i];

//             int64 goback_amount = current_frame - input_frame;  
//             JUST_LOG_INFO(
//                 "current_frame: %d, input_frame_i: %d, input_frame: %d, goback_amount: %d\n",
//                 current_frame, input_frame_i, input_frame, goback_amount
//             );
//             if (goback_amount < 0 || goback_amount >= MAX_ROLLBACK_FRAMES) {
//                 JUST_LOG_INFO("1 - %d\n", goback_amount);
//                 continue;
//             }

//             if (goback_amount == 0) {
//                 JUST_LOG_INFO("2\n");
//                 set_network_input_for_game_state(RES_game_state, input);
//             }
//             else if (!RES_saved_game_states[goback_amount].is_real) {
//                 JUST_LOG_INFO("3\n");
//                 PlayerInput* saved_input = &RES_saved_game_states[goback_amount].other_player_frame_input;
//                 bool correct_prediction = (
//                     saved_input->buttons == input.buttons
//                     && saved_input->move_stick_x == input.move_stick_x
//                     && saved_input->move_stick_y == input.move_stick_y
//                 );

//                 set_network_input_for_game_state(&RES_saved_game_states[goback_amount], input);
//                 if (!correct_prediction) {
//                     JUST_LOG_INFO("4\n");
//                     any_simulation = true;
//                     simulate_starting_from_frame = MIN(input_frame, simulate_starting_from_frame);
//                     *saved_input = input;
//                 }
//             }
//         }
//     }

//     LOCAL_server_events_offset = events_iter_end(InputPacketEvent)(&events_iter);

//     return (Option(uint32)) {
//         .is_some = any_simulation,
//         .value = simulate_starting_from_frame
//     };
// }

// void SYSTEM_FRAME_BOUNDARY_swap_event_buffers_InputPacketEvent(
//     Events(InputPacketEvent)* RES_input_packet_events
// ) {
//     events_swap_buffers(InputPacketEvent)(RES_input_packet_events);
// }

// void SYSTEM_FRAME_BOUNDARY_store_frame_game_state(
//     SpriteStore* RES_sprite_store,
//     uint32* RES_saved_frames_count,
//     GameState* RES_saved_game_states,
//     GameState* RES_game_state
// ) {
//     if (*RES_saved_frames_count == MAX_ROLLBACK_FRAMES) {
//         despawn_sprite(RES_sprite_store, RES_saved_game_states[MAX_ROLLBACK_FRAMES-1].this_player);
//         despawn_sprite(RES_sprite_store, RES_saved_game_states[MAX_ROLLBACK_FRAMES-1].other_player);
//     }
//     else {
//         *RES_saved_frames_count += 1;
//     }

//     for (uint32 i = MAX_ROLLBACK_FRAMES-1; i >= 1; i--) {
//         RES_saved_game_states[i] = RES_saved_game_states[i-1];
//     }

//     RES_saved_game_states[0] = *RES_game_state;
//     {
//         SpriteTransform transform = RES_sprite_store->transforms[RES_game_state->this_player.id];
//         Sprite sprite = RES_sprite_store->sprites[RES_game_state->this_player.id];
//         sprite.visible = false;
//         RES_saved_game_states[0].this_player = spawn_sprite(RES_sprite_store, transform, sprite);
//     }
//     {
//         SpriteTransform transform = RES_sprite_store->transforms[RES_game_state->other_player.id];
//         Sprite sprite = RES_sprite_store->sprites[RES_game_state->other_player.id];
//         sprite.visible = false;
//         RES_saved_game_states[0].other_player = spawn_sprite(RES_sprite_store, transform, sprite);
//     }
// }

// void SYSTEM_UPDATE_player_transform(
//     SpriteStore* RES_sprite_store,
//     GamepadInputs* RES_player_inputs,
//     SpriteEntityId player,
//     float32 delta_time
// ) {
//     static uint32 frame = 0;
//     static bool toggle = true;
//     static bool do_log = true;
    
//     // JUST_LOG_WARN("player[%d, %d]\n", player.id, player.generation);

//     if (!sprite_is_valid(RES_sprite_store, player)) {
//         JUST_LOG_WARN(
//             "player[%d] sprite invalid, gen: %d - store: count: %d, capacity: %d, gen: %d\n",
//             player.id, player.generation, RES_sprite_store->count, RES_sprite_store->capacity, RES_sprite_store->generations[player.id]
//         );
//         return;
//     }

//     // no need to check generation, assume no despawn
//     assert(player.generation == RES_sprite_store->generations[player.id]);

//     SpriteTransform* transform = &RES_sprite_store->transforms[player.id];
    
//     Vector2 velocity = {
//         .x = gamepad_axis_value(RES_player_inputs, GAMEPLAY_AXIS_INPUT_MOVE_X),
//         .y = gamepad_axis_value(RES_player_inputs, GAMEPLAY_AXIS_INPUT_MOVE_Y),
//     };

//     float32 speed = 150.0;
//     transform->position = Vector2Add(transform->position, Vector2Scale(velocity, speed * delta_time));

//     #if 0
//     // meta extension proposal
//     #define __MATH(...) {0}
//     transform->position = (Vector2) __MATH(
//         transform->position + (velocity * (speed * delta_time))
//     );
//     #endif

//     // JUST_LOG_INFO("player[%d] transform->position: {%0.2f, %0.2f}\n", player.id, transform->position.x, transform->position.y);
// }

// void on_write(Socket socket, SocketAddr addr, void* arg) {
//     JUST_LOG_INFO("Write Made\n");
//     Buffer* buffer = arg;
//     free(buffer);
// }

// void server_write_frame_input(uint32 frame, PlayerInput frame_input) {
//     // frame_input.move_stick_x = 100;

//     RES.input_packet.this_frame = frame;
//     RES.input_packet.num_inputs = MIN(RES.input_packet.num_inputs + 1, 3);
//     RES.input_packet.inputs[2] = RES.input_packet.inputs[1];
//     RES.input_packet.inputs[1] = RES.input_packet.inputs[0];
//     RES.input_packet.inputs[0] = frame_input;

//     Buffer* buffer = malloc_buffer(INPUT_PACKET_BYTE_SIZE);
//     pack_input_packet(buffer->bytes, RES.input_packet);

//     SocketAddr addr = {
//         .host = "127.0.0.1",
//         .port = 8080,
//     };
//     network_write_datagram(g_socket, addr, *buffer, on_write, buffer);
// }

// bool on_read(Socket socket, SocketAddr addr, BufferSlice read_buffer, void* arg) {
//     FillBuffer* read_fill = arg;

//     if (read_buffer.length == 0) {
//         free(read_fill->bytes);
//         return true; // should_remove
//     }
    
//     JUST_LOG_INFO("reading received bytes for [InputPacket: %d]\n", INPUT_PACKET_BYTE_SIZE);
//     for (usize i = 0; i < read_buffer.length; i++) {
//         *read_fill->cursor = read_buffer.bytes[i];
//         read_fill->cursor++;
//         if (filled_length(read_fill) == INPUT_PACKET_BYTE_SIZE) {
//             read_fill->cursor = read_fill->bytes;

//             InputPacketEvent event = {
//                 .packet = unpack_input_packet(read_fill->bytes),
//                 .consumed = false,
//             };

//             uint32 current_frame = atomic_load(&RES.current_frame);
//             if (current_frame - event.packet.this_frame < MAX_ROLLBACK_FRAMES) {
//                 JUST_LOG_INFO("sending event InputPacketEvent\n");
//                 events_send_single(InputPacketEvent)(&RES.input_packet_events, event);
//             }
//         }
//     }
//     return false;
// }

// // void on_connect(Socket socket, bool success, void* _arg) {
// //     if (success) {
// //         JUST_LOG_INFO("Connection Made\n");
// //         is_connected = true;
    
// //         FillBuffer* on_read_arg = malloc_fillbuffer(INPUT_PACKET_BYTE_SIZE);
// //         network_start_read_stream(socket, on_read, on_read_arg);
// //     }
// //     else {
// //         JUST_LOG_WARN("Connection Failed\n");
// //     }
// // }

// void server_connect() {
//     // g_socket = make_socket(SOCKET_TYPE_DATAGRAM);
//     // is_connected = true;
//     // FillBuffer* on_read_arg = malloc_fillbuffer(INPUT_PACKET_BYTE_SIZE);
//     // network_start_read_datagram(g_socket, on_read, on_read_arg);

//     // Socket socket = make_socket(SOCKET_TYPE_DATAGRAM);
//     // SocketAddr addr = {
//     //     .host = "127.0.0.1",
//     //     .port = 8080,
//     // };
//     // network_connect(socket, addr, on_connect, NULL);
//     // g_socket = socket;
// }

// bool CHAPTER_wait_for_connection() {
//     bool chapter_end = false;
//     while (!WindowShouldClose() && !chapter_end) {
//         float32 delta_time = GetFrameTime();
        
//         if (!connect_started && !is_connected && IsKeyPressed(KEY_SPACE)) {
//             connect_started = true;
//             server_connect();
//         }
//         if (is_connected && !just_connected && !connection_handled) {
//             just_connected = true;
//         }

//         if (just_connected) {
//             just_connected = false;
//             connection_handled = true;

//             RES.__game_state.this_player = spawn_sprite(
//                 &RES.sprite_store,
//                 (SpriteTransform) {
//                     .anchor = make_anchor(Anchor_Bottom_Mid),
//                     .position = { 0, 100 },
//                     // .use_source_size = false,
//                     .size = { 50, 50 },
//                     .scale = Vector2_From(1),
//                     .rotation = 0,
//                     .rway = Rotation_CW,
//                 },
//                 (Sprite) {
//                     .texture = new_texture_handle(WHITE_TEXTURE_HANDLE_ID),
//                     .tint = BLACK,
//                     .use_custom_source = false,
//                     // .source = get_current_frame_source(&player_state_collection),
//                     .z_index = 10,
//                     //
//                     .use_layer_system = false,
//                     // .layers = on_single_layer(1),
//                     .visible = true,
//                     .camera_visible = true,
//                 }
//             );

//             RES.__game_state.other_player = spawn_sprite(
//                 &RES.sprite_store,
//                 (SpriteTransform) {
//                     .anchor = make_anchor(Anchor_Bottom_Mid),
//                     .position = { 0, -100 },
//                     // .use_source_size = false,
//                     .size = { 50, 50 },
//                     .scale = Vector2_From(1),
//                     .rotation = 0,
//                     .rway = Rotation_CW,
//                 },
//                 (Sprite) {
//                     .texture = new_texture_handle(WHITE_TEXTURE_HANDLE_ID),
//                     .tint = WHITE,
//                     .use_custom_source = false,
//                     // .source = get_current_frame_source(&player_state_collection),
//                     .z_index = 10,
//                     //
//                     .use_layer_system = false,
//                     // .layers = on_single_layer(1),
//                     .visible = true,
//                     .camera_visible = true,
//                 }
//             );

//             chapter_end = true;
//         }

//         Color bg_color = connect_started ? YELLOW : RED;

//         BeginDrawing();
//         ClearBackground(bg_color);
//         EndDrawing();
//     }
//     return is_connected;
// }

// void run_this_frame(GameState* game_state) {
//     float32 delta_time = game_state->delta_time;

//     SYSTEM_UPDATE_player_transform(
//         &RES.sprite_store,
//         &game_state->this_player_inputs,
//         game_state->this_player,
//         delta_time
//     );
//     SYSTEM_UPDATE_player_transform(
//         &RES.sprite_store,
//         &game_state->other_player_inputs,
//         game_state->other_player,
//         delta_time
//     );
// }

// void simulate_game(uint32 simuation_start_frame) {
//     uint32 real_current_frame = RES.__game_state.current_frame;
//     uint32 goback_frames = real_current_frame - simuation_start_frame;

//     JUST_LOG_INFO("real_current_frame: %d, simuation_start_frame: %d, goback_frames: %d\n", real_current_frame, simuation_start_frame, goback_frames);
//     for (int32 i = goback_frames - 1; i >= -1 && i < MAX_ROLLBACK_FRAMES-1; i--) {
//         GameState* prev_game_state = &RES.saved_game_states[i+1];
//         GameState* this_game_state = (i == -1) ? &RES.__game_state : &RES.saved_game_states[i];

//         // JUST_LOG_INFO("this_game_state->current_frame: %d\n", this_game_state->current_frame);

//         // Reset frame
//         reset_game_state(this_game_state, prev_game_state);

//         // Simulate frame
//         run_this_frame(this_game_state);
//     }
//     RES.sprite_store.sprites[RES.__game_state.this_player.id].visible = true;
//     RES.sprite_store.sprites[RES.__game_state.other_player.id].visible = true;
// }

// bool CHAPTER_fight() {
//     Timer timer = new_timer(0.5, Timer_Repeating);
    
//     while (!WindowShouldClose()) {
//         float32 delta_time = GetFrameTime();

//         atomic_fetch_add(&RES.current_frame, 1);
//         RES.__game_state.current_frame++;
//         RES.__game_state.delta_time = delta_time;
//         RES.__game_state.is_real = false;

//         PlayerInput this_player_input = {
//             .buttons = (
//                 (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP) << 0)
//                 + (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) << 1)
//                 + (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN) << 2)
//                 + (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) << 3)
//                 + (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) << 4)
//                 + (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1) << 5)
//             ),
//             .move_stick_x = (int8) (GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X) * 127.0),
//             .move_stick_y = (int8) (GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) * 127.0),
//         };
//         handle_player_input(&RES.__game_state.this_player_inputs, this_player_input);

//         // tick_timer(&timer, delta_time);
//         // if (timer_is_finished(&timer)) {
//         // }
//         server_write_frame_input(RES.__game_state.current_frame, this_player_input);
        
//         Option(uint32) simuation_start = SYSTEM_INPUT_read_input_packets(
//             &RES.__game_state,
//             RES.saved_game_states,
//             &RES.input_packet_events
//         );

//         if (simuation_start.is_some) {
//             JUST_LOG_INFO("simulate\n");
//             simulate_game(simuation_start.value);
//         }
//         else {
//             // JUST_LOG_INFO("run\n");
//             run_this_frame(&RES.__game_state);
//         }

//         SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites(
//             &RES.camera_store,
//             &RES.sprite_store,
//             &RENDER_RES.prepared_render_sprites
//         );

//         BeginDrawing();
//         ClearBackground(GREEN);

//             SYSTEM_RENDER_sorted_sprites(
//                 &RES.texture_assets,
//                 &RES.camera_store,
//                 &RENDER_RES.prepared_render_sprites
//             );
            
//             // for (usize i = 0; i < RES.frame_id_list.count; i++) {
//             //     uint64 frame_id = RES.frame_id_list.items[i];
//             //     const char* frame_id_text = TextFormat("%llu", frame_id);
//             //     DrawText(frame_id_text, 100, 50 + (50 * i), 30, BLACK);
//             // }
//             // bool gp = IsGamepadAvailable(0);
//             // const char* gp_text = TextFormat("%d", gp);
//             // DrawText(gp_text, 100, 50 + (50 * 0), 30, BLACK);

//         EndDrawing();
        
//         SYSTEM_FRAME_BOUNDARY_swap_event_buffers_InputPacketEvent(
//             &RES.input_packet_events
//         );

//         SYSTEM_FRAME_BOUNDARY_store_frame_game_state(
//             &RES.sprite_store,
//             &RES.saved_frames_count,
//             RES.saved_game_states,
//             &RES.__game_state
//         );
//     }
//     return true;
// }

// int __main() {
//     SET_LOG_LEVEL(LOG_LEVEL_NONE);

//     InitWindow(1000, 1000, "Test");
//     SetTargetFPS(60);

//     init_network_thread();

//     // LAZY INIT
//     {
//         RES.texture_assets = new_texture_assets();
//         RES.input_packet_events = events_create(InputPacketEvent)();

//         SpriteCamera primary_camera = {
//             .camera = {
//                 .offset = {
//                     .x = 1000 / 2.0,
//                     .y = 1000 / 2.0,
//                 },
//                 .target = {0},
//                 .rotation = 0,
//                 .zoom = 1,
//             },
//             .layers = on_primary_layer(),
//             .sort_index = 0,
//         };
//         set_primary_camera(&RES.camera_store, primary_camera);
//     }

//     bool connection_success = CHAPTER_wait_for_connection();
//     if (connection_success) {
//         bool fight_end = CHAPTER_fight();
//     }
// }

// int _main() {

//     InitWindow(1000, 1000, "Test");
//     SetTargetFPS(60);

//     Camera2D camera = {0};
//     camera.zoom = 1.0;
//     camera.offset = (Vector2) {
//         .x = 1000 / 2.0,
//         .y = 1000 / 2.0,
//     };

//     Texture fire_texture = LoadTexture("test-assets\\fire.png");
//     Vector2 position = { 300, 300 };
//     Vector2 size = { 490 * 0.1, 970 * 0.1 };
//     Vector2 origin = {
//         .x = size.x * 0.5, // size.x * 0.5,
//         .y = size.y, // size.y * 0.5,
//     };
//     float32 rotation = 0.0;

//     while (!WindowShouldClose()) {
//         float32 delta_time = GetFrameTime();

//         if (!IsKeyDown(KEY_SPACE)) {
//             rotation += 20.0 * delta_time;
//             rotation -= (rotation > 360.0) ? 360.0 : 0.0;
//         }

//         if (IsKeyPressed(KEY_ENTER)) {
//             rotation = 0.0;
//         }

//         BeginDrawing();
//         ClearBackground(WHITE);
//             BeginMode2D(camera);

//                 DrawTexturePro(
//                     fire_texture,
//                     (Rectangle) {0, 0, fire_texture.width, fire_texture.height},
//                     (Rectangle) {
//                         .x = position.x,
//                         .y = position.y,
//                         .width = size.x,
//                         .height = size.y,
//                     },
//                     origin,
//                     rotation,
//                     WHITE
//                 );

//                 DrawTexturePro(
//                     fire_texture,
//                     (Rectangle) {0, 0, -fire_texture.width, fire_texture.height},
//                     (Rectangle) {
//                         .x = -position.x,
//                         .y = position.y,
//                         .width = size.x,
//                         .height = size.y,
//                     },
//                     (Vector2) {0, 0},
//                     0,
//                     WHITE
//                 );

//                 DrawTexturePro(
//                     fire_texture,
//                     (Rectangle) {0, 0, fire_texture.width, -fire_texture.height},
//                     (Rectangle) {
//                         .x = position.x,
//                         .y = -position.y,
//                         .width = size.x,
//                         .height = size.y,
//                     },
//                     (Vector2) {0, 0},
//                     0,
//                     WHITE
//                 );

//                 DrawTexturePro(
//                     fire_texture,
//                     (Rectangle) {0, 0, -fire_texture.width, -fire_texture.height},
//                     (Rectangle) {
//                         .x = -position.x,
//                         .y = -position.y,
//                         .width = size.x,
//                         .height = size.y,
//                     },
//                     (Vector2) {0, 0},
//                     0,
//                     WHITE
//                 );

//                 DrawCircleV(position, 10, LIME); // tex origin
//                 DrawCircle(0, 0, 10, RED); // origin

//             EndMode2D();
//         EndDrawing();
//     }

//     return 0;
// }

void echo_on_write(WriteContext context, void* arg) {
    byte* msg_buffer = arg;
    JUST_LOG_INFO("Write Made\n");
    // static uint64 count = 0;
    // count++;
    // if (count == 2) {
    //     PANIC("WRITE\n");
    // }
    free(msg_buffer);
}

bool server_on_read(ReadContext context, BufferSlice read_buffer, void* arg) {
    JUST_LOG_INFO("Read: %d\n", read_buffer.length);
    // static uint64 count = 0;
    // count++;
    // if (count == 2) {
    //     PANIC("READ\n");
    // }
    Buffer echo_msg = buffer_clone(read_buffer);
    network_write_buffer(context.socket, echo_msg, echo_on_write, echo_msg.bytes);
    return true;
}

void server_on_accept(uint32 server_id, Socket socket, void* arg) {
    network_start_read(socket, server_on_read, NULL);
}

int main() {
    SET_LOG_LEVEL(LOG_LEVEL_INFO);
    SET_LOG_LEVEL(LOG_LEVEL_TRACE);

    InitWindow(1000, 1000, "Test");
    SetTargetFPS(5);

    init_network_thread();

    Timer start_timer = new_timer(5, Timer_NonRepeating);
    Timer timer = new_timer(10, Timer_Repeating);
    while (!WindowShouldClose()) {
        float32 delta_time = GetFrameTime();

        if (!timer_is_finished(&start_timer)) {
            tick_timer(&start_timer, delta_time);
            if (timer_is_finished(&start_timer)) {
                SocketAddr server_addr = {
                    .host = "127.0.0.1",
                    .port = 8080,
                };
                network_listen(server_addr, NETWORK_PROTOCOL_TCP, 0, server_on_accept, NULL);
                JUST_DEV_MARK();
            }
        }

        tick_timer(&timer, delta_time);
        if (timer_is_finished(&timer)) {
            JUST_LOG_INFO("-- ALIVE --\n");
        }

        BeginDrawing();
        EndDrawing();
    }
}