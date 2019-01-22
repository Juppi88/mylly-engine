#include "inputhook.h"
#include "mgui.h"

// -------------------------------------------------------------------------------------------------

bool mgui_handle_keyboard_event(input_event_t type, uint32_t key)
{
	return true;
}

bool mgui_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							 mouse_button_t button, mouse_wheel_t wheel)
{
	if (type == INPUT_MOUSE_BUTTON_DOWN) {

		// User clicked somewhere. Find out whether there's a widget under the cursor and if so,
		// move focus to said widget.
		widget_t *widget = mgui_get_widget_at_position(vec2i(x, y));

		if (widget != NULL) {
			
			mgui_set_focused_widget(widget);
			return false;
		}
	}

	return true;
}
