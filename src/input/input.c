#include "input.h"

// -- Keyboard

bool key_just_pressed(KeyInputs* key_inputs, uint32 key) {
    return key_inputs->keys[key] == KEY_STATE_PRESSED;
}
bool key_is_repeated(KeyInputs* key_inputs, uint32 key) {
    return key_inputs->keys[key] == KEY_STATE_REPEATED;
}
bool key_is_released(KeyInputs* key_inputs, uint32 key) {
    return key_inputs->keys[key] == KEY_STATE_RELEASED;
}
bool key_is_up(KeyInputs* key_inputs, uint32 key) {
    return key_inputs->keys[key] >= KEY_STATE_PRESSED;
}
bool key_is_down(KeyInputs* key_inputs, uint32 key) {
    return key_inputs->keys[key] <= KEY_STATE_RELEASED;
}

void update_key_state(KeyInputs* key_inputs, uint32 key, bool state) {
    KeyState* key_state = &key_inputs->keys[key];
    switch (*key_state) {
    case KEY_STATE_NULL:
    case KEY_STATE_RELEASED:
        *key_state = branchless_if(state, KEY_STATE_PRESSED, KEY_STATE_NULL);
        break;
    case KEY_STATE_PRESSED:
    case KEY_STATE_REPEATED:
        *key_state = branchless_if(state, KEY_STATE_REPEATED, KEY_STATE_RELEASED);
        break;
    }
}

void set_key_repeated(KeyInputs* key_inputs, uint32 key) {
    key_inputs->keys[key] = KEY_STATE_REPEATED;
}

void SYSTEM_INPUT_get_keyboard_inputs(KeyInputs* RES_key_inputs) {
    for (uint32 key = 0; key < MAX_KEYBOARD_KEYS; key++) {
        if (IsKeyPressedRepeat(key)) {
            set_key_repeated(RES_key_inputs, key);
        }
        else {
            update_key_state(RES_key_inputs, key, IsKeyDown(key));
        }
    }
}

// -- Gamepad

bool gamepad_button_just_pressed(GamepadInputs* gamepad_inputs, uint32 button) {
    return gamepad_inputs->buttons[button] == KEY_STATE_PRESSED;
}
bool gamepad_button_is_repeated(GamepadInputs* gamepad_inputs, uint32 button) {
    return gamepad_inputs->buttons[button] == KEY_STATE_REPEATED;
}
bool gamepad_button_is_released(GamepadInputs* gamepad_inputs, uint32 button) {
    return gamepad_inputs->buttons[button] == KEY_STATE_RELEASED;
}
bool gamepad_button_is_up(GamepadInputs* gamepad_inputs, uint32 button) {
    return gamepad_inputs->buttons[button] >= KEY_STATE_PRESSED; // KEY_STATE_PRESSED || KEY_STATE_REPEATED
}
bool gamepad_button_is_down(GamepadInputs* gamepad_inputs, uint32 button) {
    return gamepad_inputs->buttons[button] <= KEY_STATE_RELEASED; // KEY_STATE_NULL || KEY_STATE_RELEASED
}
float32 gamepad_axis_value(GamepadInputs* gamepad_inputs, uint32 axis) {
    return gamepad_inputs->axis_values[axis];
}
float32 gamepad_axis_delta(GamepadInputs* gamepad_inputs, uint32 axis) {
    return gamepad_inputs->axis_values[axis] - gamepad_inputs->prev_axis_values[axis];
}

void update_gamepad_button_state(GamepadInputs* gamepad_inputs, uint32 button, bool state) {
    KeyState* button_state = &gamepad_inputs->buttons[button];
    switch (*button_state) {
    case KEY_STATE_NULL:
    case KEY_STATE_RELEASED:
        *button_state = branchless_if(state, KEY_STATE_PRESSED, KEY_STATE_NULL);
        break;
    case KEY_STATE_PRESSED:
    case KEY_STATE_REPEATED:
        *button_state = branchless_if(state, KEY_STATE_REPEATED, KEY_STATE_RELEASED);
        break;
    }
}


void update_gamepad_axis_value(GamepadInputs* gamepad_inputs, uint32 axis, float32 value) {
    gamepad_inputs->prev_axis_values[axis] = gamepad_inputs->axis_values[axis];
    gamepad_inputs->axis_values[axis] = value;
}