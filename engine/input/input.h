#pragma once
#ifndef __INPUT_H
#define __INPUT_H

#include "input/keybind.h"
#include "core/defines.h"

// --------------------------------------------------------------------------------

typedef enum input_event_t {
	INPUT_CHARACTER, // Input a character
	INPUT_KEY_UP, // A keyboard button is released
	INPUT_KEY_DOWN, // A keyboard button is pressed
	INPUT_MOUSE_MOVE, // Mouse movement
	INPUT_MOUSE_WHEEL, // Mouse scroll
	INPUT_LBUTTON_UP, // Left mouse button released
	INPUT_LBUTTON_DOWN, // Left mouse button pressed
	INPUT_MBUTTON_UP, // Middle mouse button released
	INPUT_MBUTTON_DOWN, // Middle mouse button pressed
	INPUT_RBUTTON_UP, // Right mouse button released
	INPUT_RBUTTON_DOWN, // Right mouse button pressed
	NUM_INPUT_EVENTS
} input_event_t;

typedef enum mouse_button_t {
	MOUSE_NONE,
	MOUSE_LBUTTON,
	MOUSE_MBUTTON,
	MOUSE_RBUTTON,
	MOUSE_FORCE_DWORD = 0x7FFFFFFF
} mouse_button_t;

typedef enum mouse_wheel_t {
	MWHEEL_STATIONARY,
	MWHEEL_UP,
	MWHEEL_DOWN,
} mouse_wheel_t;

// --------------------------------------------------------------------------------

bool input_handle_keyboard_event(input_event_t type, uint32_t key);

bool input_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							  mouse_button_t button, mouse_wheel_t wheel);

#endif
