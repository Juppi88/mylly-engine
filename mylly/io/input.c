#include "input.h"
#include "platform/inputhook.h"
#include "core/memory.h"
#include "core/time.h"
#include "collections/list.h"
#include "mgui/uiinput.h"

// -------------------------------------------------------------------------------------------------

// Cached mouse cursor position.
static int16_t mouse_x, mouse_y;

// Reference counter for mouse cursor. When <= 0, cursor is not visible.
static int cursor_reference_count = 0;

// Stores the key symbols of the virtual button binds.
static uint32_t button_symbols[256];

// -------------------------------------------------------------------------------------------------

typedef struct keybind_t {

	list_entry(keybind_t);

	uint32_t key_symbol; // The symbol this handler is bound to (defined in 'input/keys.h')
	keybind_handler_t handler; // The handler method
	void *context; // Event handler context

} keybind_t;

// List of all active keybinds.
list_t(keybind_t) keybinds;

// -------------------------------------------------------------------------------------------------

static bool input_process_keybinds(uint32_t key_symbol, bool pressed);

// -------------------------------------------------------------------------------------------------

void input_initialize(void)
{
	// Hide the cursor until requested to be visible.
	input_sys_toggle_cursor(false);
}

void input_shutdown(void)
{
	// Remove all keybinds which are still active.
	keybind_t *bind, *tmp;

	list_foreach_safe(keybinds, bind, tmp) {
		DESTROY(bind);
	}
}

bool input_is_key_down(uint32_t key_symbol)
{
	return (input_sys_get_key_pressed_frame(key_symbol) != 0);
}

void input_bind_button(button_code_t button, uint32_t key_symbol)
{
	button_symbols[button] = key_symbol;
}

bool input_is_button_down(button_code_t button)
{
	uint32_t key_symbol = button_symbols[button];

	if (key_symbol == 0) {

		// Button has not been bound.
		return false;
	}

	return (input_sys_get_key_pressed_frame(key_symbol) != 0);
}

bool input_get_button_pressed(button_code_t button)
{
	uint32_t key_symbol = button_symbols[button];
	uint32_t frame = get_time().frame_count;

	if (key_symbol == 0) {

		// Button has not been bound.
		return false;
	}

	return (input_sys_get_key_pressed_frame(key_symbol) == frame);
}

bool input_get_button_released(button_code_t button)
{
	uint32_t key_symbol = button_symbols[button];
	uint32_t frame = get_time().frame_count;

	if (key_symbol == 0) {

		// Button has not been bound.
		return false;
	}

	return (input_sys_get_key_released_frame(key_symbol) == frame);
}

void input_bind_key(uint32_t key_symbol, keybind_handler_t method, void *context)
{
	if (method == NULL) {
		return;
	}

	NEW(keybind_t, bind);

	bind->key_symbol = key_symbol;
	bind->handler = method;
	bind->context = context;

	list_push(keybinds, bind);
}

void input_unbind_key(uint32_t key_symbol, keybind_handler_t method, void *context)
{
	keybind_t *bind, *tmp;

	list_foreach_safe(keybinds, bind, tmp) {

		// Remove all bind with the matching key and handler method.
		if (bind->key_symbol == key_symbol &&
			bind->handler == method &&
			bind->context == context) {

			list_remove(keybinds, bind);
			DESTROY(bind);
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

void input_toggle_cursor(bool visible)
{
	if (visible) {
		cursor_reference_count++;
	}
	else {
		cursor_reference_count--;
	}

	if (cursor_reference_count == 0 && !visible) { // Cursor just turned invisible
		input_sys_toggle_cursor(false);
	}
	else if (cursor_reference_count == 1 && visible) { // Cursor just turned visible
		input_sys_toggle_cursor(true);
	}
}

bool input_handle_keyboard_event(input_event_t type, uint32_t key)
{
	// Call MGUI's input hook.
	if (!mgui_handle_keyboard_event(type, key)) {
		return false;
	}

	// Process keybinds unless MGUI blocks them.
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

	if (!mgui_handle_mouse_event(type, x, y, button, wheel)) {
		return false;
	}

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
			if (!bind->handler(key_symbol, pressed, bind->context)) {
				return false;
			}
		}
	}

	return true;
}
