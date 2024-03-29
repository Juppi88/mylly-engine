#include "uiinput.h"
#include "mgui.h"
#include "widget.h"

// -------------------------------------------------------------------------------------------------

static vec2i_t cursor_position;

// -------------------------------------------------------------------------------------------------

static void mgui_widget_keyboard_event(input_event_t type, uint32_t key);
static void mgui_widget_mouse_event(input_event_t type, int16_t x, int16_t y,
                                    mouse_button_t button, mouse_wheel_t wheel);

// -------------------------------------------------------------------------------------------------

bool mgui_handle_keyboard_event(input_event_t type, uint32_t key)
{
	widget_t *focused;
	bool result = true;

	switch (type) {
		case INPUT_CHARACTER:

			focused = mgui_get_focused_widget();
			if (focused != NULL &&
				focused->callbacks->on_character_injected != NULL &&
				!focused->callbacks->on_character_injected(focused, key)) {

				result = false;
			}

			break;

		case INPUT_KEY_DOWN:
		case INPUT_KEY_UP:

			focused = mgui_get_focused_widget();

			if (focused != NULL &&
				focused->callbacks->on_key_pressed != NULL &&
				!focused->callbacks->on_key_pressed(focused, key, type == INPUT_KEY_DOWN)) {

				result = false;
			}

			break;

		default:
			break;
	}

	// Call custom input event handler on focused widget.
	mgui_widget_keyboard_event(type, key);

	return result;
}

bool mgui_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							 mouse_button_t button, mouse_wheel_t wheel)
{
	bool result = true;

	// Convert coordinates to virtual UI resolution.
	mgui_convert_to_ui_coordinates(&x, &y);

	switch (type) {
		case INPUT_MOUSE_BUTTON_DOWN:
		{
			cursor_position = vec2i(x, y);

			// User clicked somewhere. Find out whether there's a widget under the cursor and if so,
			// move focus to said widget.
			widget_t *widget = mgui_get_widget_at_position(cursor_position);

			mgui_set_focused_widget(widget);
			mgui_set_dragged_widget(widget);
			mgui_set_pressed_widget(widget, x, y);

			if (widget != NULL) {
				result = false;
			}
			
			break;
		}
		case INPUT_MOUSE_BUTTON_UP:
		{
			mgui_set_dragged_widget(NULL);
			mgui_set_pressed_widget(NULL, x, y);

			break;
		}
		case INPUT_MOUSE_MOVE:
		{
			widget_t *dragged = mgui_get_dragged_widget();

			// If dragging a widget, move it with the cursor (unless it has a special callback
			// for dragging events).
			if (dragged != NULL) {

				if (dragged->callbacks->on_dragged != NULL) {
					dragged->callbacks->on_dragged(dragged, x, y);
				}
				else {

					int16_t dx = x - cursor_position.x;
					int16_t dy = y - cursor_position.y;

					vec2i_t position = vec2i(dragged->position.x + dx, dragged->position.y + dy);
					widget_set_position(dragged, position);
				}
			}
			else {

				widget_t *hovered = mgui_get_widget_at_position(cursor_position);
				mgui_set_hovered_widget(hovered);

				// When the cursor is moved out of a pressed widget, reset the widget's state.
				if (hovered != mgui_get_pressed_widget()) {
					mgui_set_pressed_widget(NULL, x, y);
				}
			}

			// Update cursor position.
			cursor_position = vec2i(x, y);

			break;
		}
		default:
			break;
	}

	// Call custom input event handler on focused widget.
	mgui_widget_mouse_event(type, x, y, button, wheel);

	return result;
}

static void mgui_widget_keyboard_event(input_event_t type, uint32_t key)
{
	// Relay keyboard event to the focused widget if it has a custom override event handler.
	widget_t *widget = mgui_get_focused_widget();

	if (widget == NULL ||
		widget->input_handler == NULL) {

		return;
	}

	widget_event_t event;

	event.widget = widget;
	event.type = type;
	event.keyboard.key = key;

	widget->input_handler(&event);
}

static void mgui_widget_mouse_event(input_event_t type, int16_t x, int16_t y,
                                    mouse_button_t button, mouse_wheel_t wheel)
{
	// Relay mouse event to the focused widget if it has a custom override event handler.
	widget_t *widget = mgui_get_focused_widget();

	if (widget == NULL ||
		widget->input_handler == NULL) {

		return;
	}

	widget_event_t event;

	event.widget = widget;
	event.type = type;
	event.mouse.x = x;
	event.mouse.y = y;
	event.mouse.button = button;
	event.mouse.wheel = wheel;

	widget->input_handler(&event);
}
