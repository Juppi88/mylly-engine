#include "input.h"
#include "platform/input.h"
#include "core/time.h"

// --------------------------------------------------------------------------------

// Cached mouse cursor position.
static int16_t mouse_x, mouse_y;

// Stores the key symbols of the virtual button binds.
static uint32_t button_symbols[256];

// --------------------------------------------------------------------------------

void input_bind_button(button_t button, uint32_t key_symbol)
{
	button_symbols[button] = key_symbol;
}

bool input_is_button_down(button_t button)
{
	uint32_t key_symbol = button_symbols[button];

	if (key_symbol == 0) {

		// Button has not been bound.
		return false;
	}

	return (input_sys_get_key_pressed_frame(key_symbol) != 0);
}

bool input_get_button_pressed(button_t button)
{
	uint32_t key_symbol = button_symbols[button];
	uint32_t frame = get_time().frame_count;

	if (key_symbol == 0) {

		// Button has not been bound.
		return false;
	}

	return (input_sys_get_key_pressed_frame(key_symbol) == frame);
}

bool input_get_button_released(button_t button)
{
	uint32_t key_symbol = button_symbols[button];
	uint32_t frame = get_time().frame_count;

	if (key_symbol == 0) {

		// Button has not been bound.
		return false;
	}

	return (input_sys_get_key_released_frame(key_symbol) == frame);
}

void input_get_cursor_position(uint16_t *x, uint16_t *y)
{
	if (x == NULL || y == NULL) {
		return;
	}

	*x = mouse_x;
	*y = mouse_y;
}

void input_set_cursor_position(uint16_t x, uint16_t y)
{
	input_sys_warp_cursor(x, y);

	mouse_x = x;
	mouse_y = y;
}

bool input_handle_keyboard_event(input_event_t type, uint32_t key)
{
	return true;
}

bool input_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							  mouse_button_t button, mouse_wheel_t wheel)
{
	mouse_x = x;
	mouse_y = y;

	return true;
}
