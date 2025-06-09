#pragma once

#include "base.h"

// same MAX definitions as in raylib
#define MAX_KEYBOARD_KEYS 512
// #define MAX_GAMEPADS 4
#define MAX_GAMEPAD_BUTTONS 32
#define MAX_GAMEPAD_AXES 10

typedef enum {
    KEY_STATE_NULL = 0,
    KEY_STATE_RELEASED = 1,
    KEY_STATE_PRESSED = 2,
    KEY_STATE_REPEATED = 3,
} KeyState;

typedef struct {
    KeyState keys[MAX_KEYBOARD_KEYS];
} KeyInputs;

bool key_just_pressed(KeyInputs* key_inputs, uint32 key);
bool key_is_repeated(KeyInputs* key_inputs, uint32 key);
bool key_is_released(KeyInputs* key_inputs, uint32 key);
bool key_is_up(KeyInputs* key_inputs, uint32 key);
bool key_is_down(KeyInputs* key_inputs, uint32 key);

void update_key_state(KeyInputs* key_inputs, uint32 key, bool state);
void set_key_repeated(KeyInputs* key_inputs, uint32 key);

typedef struct {
    KeyState buttons[MAX_GAMEPAD_BUTTONS];
    float32 axis_values[MAX_GAMEPAD_AXES];
    float32 prev_axis_values[MAX_GAMEPAD_AXES];
} GamepadInputs;

bool gamepad_button_just_pressed(GamepadInputs* gamepad_inputs, uint32 button);
bool gamepad_button_is_repeated(GamepadInputs* gamepad_inputs, uint32 button);
bool gamepad_button_is_released(GamepadInputs* gamepad_inputs, uint32 button);
bool gamepad_button_is_up(GamepadInputs* gamepad_inputs, uint32 button);
bool gamepad_button_is_down(GamepadInputs* gamepad_inputs, uint32 button);
float32 gamepad_axis_value(GamepadInputs* gamepad_inputs, uint32 axis);
float32 gamepad_axis_delta(GamepadInputs* gamepad_inputs, uint32 axis);

void update_gamepad_button_state(GamepadInputs* gamepad_inputs, uint32 button, bool state);
void update_gamepad_axis_value(GamepadInputs* gamepad_inputs, uint32 axis, float32 value);
