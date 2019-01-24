#include "inputhook.h"
#include "mgui.h"
#include "widget.h"

// -------------------------------------------------------------------------------------------------

static vec2i_t cursor_position;

// -------------------------------------------------------------------------------------------------

bool mgui_handle_keyboard_event(input_event_t type, uint32_t key)
{
	return true;
}

bool mgui_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							 mouse_button_t button, mouse_wheel_t wheel)
{
	switch (type) {
		case INPUT_MOUSE_BUTTON_DOWN:
		{
			cursor_position = vec2i(x, y);

			// User clicked somewhere. Find out whether there's a widget under the cursor and if so,
			// move focus to said widget.
			widget_t *widget = mgui_get_widget_at_position(cursor_position);

			if (widget != NULL) {

				mgui_set_focused_widget(widget);
				mgui_set_dragged_widget(widget);

				return false;
			}
			break;
		}
		case INPUT_MOUSE_BUTTON_UP:
		{
			mgui_set_dragged_widget(NULL);
			break;
		}
		case INPUT_MOUSE_MOVE:
		{
			widget_t *dragged = mgui_get_dragged_widget();

			// If dragging a widget, move it with the cursor.
			if (dragged != NULL) {

				int16_t dx = x - cursor_position.x;
				int16_t dy = y - cursor_position.y;

				vec2i_t position = vec2i(dragged->position.x + dx, dragged->position.y + dy);
				widget_set_position(dragged, position);
			}

			// Update cursor position.
			cursor_position = vec2i(x, y);

			break;
		}
		default:
			break;
	}

	return true;
}
