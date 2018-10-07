#include "input.h"
#include "input/input.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static uint32_t modifier_flags = 0;

// --------------------------------------------------------------------------------

bool input_process_messages(void *params)
{
	XEvent *event = (XEvent *)params;

	XKeyEvent* key_event;
	XButtonEvent* button_event;
	XMotionEvent* motion_event;
	char buffer[20];
	KeySym sym;
	uint32_t code;
	int16_t mouse_x, mouse_y;

	switch (event->type) {

		case KeyPress: {
			key_event = (XKeyEvent *)event;

			// Store modifier flags.
			modifier_flags = key_event->state;

			// Get the symbol (key code) for the pressed button.
			XLookupString(key_event, buffer, sizeof(buffer), &sym, NULL);
			code = (uint32_t)sym;

			// Handle key down event and related binds.
			if (!input_handle_keyboard_event(INPUT_KEY_DOWN, code)) {
				return false;
			}

			// A fix to make Windows and Linux hooks compatible:
			// Convert lowercase characters to upper case before processing hooks.
			if (code >= 'a' && code <= 'z') {
				code -= ('a' - 'A');
			}

			// Handle character event and related bindings.
			if (*buffer != 0) {
				return input_handle_keyboard_event(INPUT_CHARACTER, *buffer);
			}
			
			return true;
		}

		case KeyRelease: {
			key_event = (XKeyEvent *)event;

			modifier_flags = key_event->state;

			XLookupString(key_event, buffer, sizeof(buffer), &sym, NULL);
			code = (uint32_t)sym;

			// Handle key up event and related binds.
			return input_handle_keyboard_event(INPUT_KEY_UP, code);
		}

		case ButtonPress: {
			button_event = (XButtonEvent *)event;

			mouse_x = (int16_t)button_event->x;
			mouse_y = (int16_t)button_event->y;

			switch (button_event->button) {

			case Button1:
				// Left mouse button
				XGrabPointer(button_event->display, button_event->window, false,
							 ButtonPressMask|ButtonReleaseMask|PointerMotionMask|FocusChangeMask|
							 EnterWindowMask|LeaveWindowMask,
							 GrabModeAsync, GrabModeAsync, button_event->window, None, CurrentTime);

				// Handle mouse button event and related binds.
				return input_handle_mouse_event(INPUT_LBUTTON_DOWN, mouse_x, mouse_y,
												MOUSE_LBUTTON, MWHEEL_STATIONARY);

			case Button3:
				// Right mouse button
				return input_handle_mouse_event(INPUT_RBUTTON_DOWN, mouse_x, mouse_y,
												MOUSE_RBUTTON, MWHEEL_STATIONARY);
				
			case Button2:
				// Middle mouse button (wheel)
				return input_handle_mouse_event(INPUT_MBUTTON_DOWN, mouse_x, mouse_y,
												MOUSE_MBUTTON, MWHEEL_STATIONARY);
				
			case Button4:
				// Mouse wheel scroll up
				return input_handle_mouse_event(INPUT_MOUSE_WHEEL, mouse_x, mouse_y,
												MOUSE_NONE, MWHEEL_UP);

			case Button5:
				// Mouse wheel scroll up
				return input_handle_mouse_event(INPUT_MOUSE_WHEEL, mouse_x, mouse_y,
												MOUSE_NONE, MWHEEL_DOWN);
			}

			return true;
		}

		case ButtonRelease: {
			button_event = (XButtonEvent *)event;

			mouse_x = (int16_t)button_event->x;
			mouse_y = (int16_t)button_event->y;

			switch (button_event->button) {

			case Button1:
				// Left mouse button
				XUngrabPointer(button_event->display, CurrentTime);

				return input_handle_mouse_event(INPUT_LBUTTON_UP, mouse_x, mouse_y,
												MOUSE_LBUTTON, MWHEEL_STATIONARY);

			case Button3:
				// Right mouse button
				return input_handle_mouse_event(INPUT_RBUTTON_UP, mouse_x, mouse_y,
										 MOUSE_RBUTTON, MWHEEL_STATIONARY);
			case Button2:
				// Middle mouse button (wheel)
				return input_handle_mouse_event(INPUT_MBUTTON_UP, mouse_x, mouse_y,
												MOUSE_MBUTTON, MWHEEL_STATIONARY);
			}

			return true;
		}

		case MotionNotify: {
			motion_event = (XMotionEvent *)event;

			mouse_x = (int16_t)motion_event->x;
			mouse_y = (int16_t)motion_event->y;

			return input_handle_mouse_event(INPUT_MOUSE_MOVE, mouse_x, mouse_y,
											MOUSE_NONE, MWHEEL_STATIONARY);
		}
	}
	return true;
}
