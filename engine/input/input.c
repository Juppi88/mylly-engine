#include "input.h"
#include "platform/input.h"
#include "core/memory.h"
#include "core/time.h"
#include "collections/list.h"

// -------------------------------------------------------------------------------------------------

// Cached mouse cursor position.
static int16_t mouse_x, mouse_y;

// Stores the key symbols of the virtual button binds.
static uint32_t button_symbols[256];

// -------------------------------------------------------------------------------------------------

typedef struct keybind_t {

	list_entry(keybind_t);

	uint32_t key_symbol; // The symbol this handler is bound to (defined in 'input/keys.h').
	keybind_handler_t handler; // The handler method.

} keybind_t;

// List of all active keybinds.
list_t(keybind_t) keybinds;

static bool input_process_keybinds(uint32_t key_symbol, bool pressed);

// -------------------------------------------------------------------------------------------------

void input_initialize(void)
{
}

void input_shutdown(void)
{
	// Remove all keybinds which are still active.
	keybind_t *bind, *tmp;

	list_foreach_safe(keybinds, bind, tmp) {
		DELETE(bind);
	}
}

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

void input_bind_key(uint32_t key_symbol, keybind_handler_t method)
{
	if (method == NULL) {
		return;
	}

	NEW(keybind_t, bind);

	bind->key_symbol = key_symbol;
	bind->handler = method;

	list_push(keybinds, bind);
}

void input_unbind_key(uint32_t key_symbol, keybind_handler_t method)
{
	keybind_t *bind, *tmp;

	list_foreach_safe(keybinds, bind, tmp) {

		// Remove all bind with the matching key and handler method.
		if (bind->key_symbol == key_symbol &&
			bind->handler == method) {

			list_remove(keybinds, bind);
			DELETE(bind);
		}
	}
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
	if (type == INPUT_KEY_UP ||
		type == INPUT_KEY_DOWN) {

		if (!input_process_keybinds(key, type == INPUT_KEY_DOWN)) {
			return false;
		}
	}

	return true;
}

bool input_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							  mouse_button_t button, mouse_wheel_t wheel)
{
	mouse_x = x;
	mouse_y = y;

	if (type == INPUT_MOUSE_BUTTON_UP ||
		type == INPUT_MOUSE_BUTTON_DOWN) {

		if (!input_process_keybinds(button, type == INPUT_MOUSE_BUTTON_DOWN)) {
			return false;
		}
	}

	return true;
}

static bool input_process_keybinds(uint32_t key_symbol, bool pressed)
{
	keybind_t *bind;

	list_foreach(keybinds, bind) {

		// Call all handlers bound to the key symbol.
		if (bind->key_symbol == key_symbol) {

			// Stop processing further binds if a bind returns false.
			if (!bind->handler(key_symbol, pressed)) {
				return false;
			}
		}
	}

	return true;
}
