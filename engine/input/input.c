#include "input.h"

bool input_handle_keyboard_event(input_event_t type, uint32_t key)
{
	printf("KYBRD: %u %u\n", type, key);
	return true;
}

bool input_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							  mouse_button_t button, mouse_wheel_t wheel)
{
	printf("MOUSE: (%d,%d) %u %u %u\n", x, y, type, button, wheel);
	return true;
}
