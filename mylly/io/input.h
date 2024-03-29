#pragma once
#ifndef __INPUT_H
#define __INPUT_H

#include "core/defines.h"
#include "io/keys.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

// Mouse buttons. Can be used as key symbols alongside those defined in 'input/keys.h'.
typedef enum mouse_button_t {

	MOUSE_NONE,
	MOUSE_LEFT = MKEY_LMB,
	MOUSE_MIDDLE = MKEY_MMB,
	MOUSE_RIGHT = MKEY_RMB,
	MOUSE_FORCE_DWORD = 0x7FFFFFFF

} mouse_button_t;

// -------------------------------------------------------------------------------------------------

// Mouse wheel events.
typedef enum mouse_wheel_t {

	MWHEEL_STATIONARY,
	MWHEEL_UP,
	MWHEEL_DOWN,
	
} mouse_wheel_t;

// -------------------------------------------------------------------------------------------------

// Input system initialization and de-initialization.
void input_initialize(void);
void input_shutdown(void);

// Raw key states.
bool input_is_key_down(uint32_t key_symbol);

// -------------------------------------------------------------------------------------------------

// Bind an arbitrary button index to a key symbol from 'input/keys.h'. The value can then be used
// to query the state of a button without hardcoding the symbol into the code. Setting the key
// symbol to 0 will unbind the symbol.
typedef uint16_t button_code_t;

void input_bind_button(button_code_t button, uint32_t key_symbol);

// Returns whether the button is currently pressed down.
bool input_is_button_down(button_code_t button);

// Returns whether the button was just pressed down or released.
bool input_get_button_pressed(button_code_t button);
bool input_get_button_released(button_code_t button);

// -------------------------------------------------------------------------------------------------

// Bind keyboard keys and mouse buttons to handler methods which are called every time the button is
// pressed or released.
#define KEYBIND_HANDLER(x) static bool x(uint32_t key, bool pressed, void *context)

typedef bool (*keybind_handler_t)(uint32_t key, bool pressed, void *context);

void input_bind_key(uint32_t key_symbol, keybind_handler_t method, void *context);
void input_unbind_key(uint32_t key_symbol, keybind_handler_t method, void *context);

// -------------------------------------------------------------------------------------------------

// Retrieve and set cursor position.
void input_get_cursor_position(uint16_t *x, uint16_t *y);
void input_set_cursor_position(uint16_t x, uint16_t y);

// Toggle cursor visibility by incrementing or decrementing an internal cursor reference counter.
void input_toggle_cursor(bool visible);

// -------------------------------------------------------------------------------------------------

typedef enum input_event_t {

	INPUT_CHARACTER, // Input a character
	INPUT_KEY_UP, // A keyboard button is released
	INPUT_KEY_DOWN, // A keyboard button is pressed
	INPUT_MOUSE_MOVE, // Mouse movement
	INPUT_MOUSE_WHEEL, // Mouse scroll
	INPUT_MOUSE_BUTTON_UP, // Mouse button released
	INPUT_MOUSE_BUTTON_DOWN, // Mouse button pressed
	NUM_INPUT_EVENTS

} input_event_t;

// Internal handler for raw keyboard events.
bool input_handle_keyboard_event(input_event_t type, uint32_t key);

// Internal handler for raw mouse events.
bool input_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							  mouse_button_t button, mouse_wheel_t wheel);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
