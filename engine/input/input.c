#include "input.h"
#include "platform/input.h"

static int16_t mouse_x, mouse_y;

// --------------------------------------------------------------------------------

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
	input_warp_cursor(x, y);

	mouse_x = x;
	mouse_y = y;
}

bool input_handle_keyboard_event(input_event_t type, uint32_t key)
{
	//printf("KYBRD: %u %u\n", type, key);
	return true;
}

bool input_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							  mouse_button_t button, mouse_wheel_t wheel)
{
	//printf("MOUSE: (%d,%d) %u %u %u\n", x, y, type, button, wheel);

	mouse_x = x;
	mouse_y = y;

	return true;
}
