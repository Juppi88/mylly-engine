#include "input.h"
#include "window.h"
#include "input/input.h"
#include "core/time.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// --------------------------------------------------------------------------------

#define KEY_CODES 256 // Number of key codes that a keyboard can have

// Modifier keys being pressed.
static uint32_t modifier_flags = 0;

// An array containing the specific frame count when a key is pressed down or released.
// If a key is currently not pressed down, the frame is set to 0.
// Values < 8 are reserved for mouse buttons.
static uint32_t key_pressed_frames[KEY_CODES];
static uint32_t key_released_frames[KEY_CODES];

// --------------------------------------------------------------------------------

bool input_sys_process_messages(void *params)
{
	XEvent *event = (XEvent *)params;

	XKeyEvent* key_event;
	XButtonEvent* button_event;
	XMotionEvent* motion_event;
	char buffer[20];
	KeySym sym;
	uint32_t code;
	int16_t mouse_x, mouse_y;

	uint32_t frame = get_time().frame_count;

	switch (event->type) {

		case KeyPress: {
			key_event = (XKeyEvent *)event;

			// Store the frame the button is pressed.
			key_pressed_frames[key_event->keycode] = frame;
			key_released_frames[key_event->keycode] = 0;

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

			// Key is released, update frame counts.
			key_pressed_frames[key_event->keycode] = 0;
			key_released_frames[key_event->keycode] = frame;

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
				key_pressed_frames[MOUSE_LBUTTON] = frame;
				key_released_frames[MOUSE_LBUTTON] = 0;

				XGrabPointer(button_event->display, button_event->window, false,
							 ButtonPressMask|ButtonReleaseMask|PointerMotionMask|FocusChangeMask|
							 EnterWindowMask|LeaveWindowMask,
							 GrabModeAsync, GrabModeAsync, button_event->window, None, CurrentTime);

				// Handle mouse button event and related binds.
				return input_handle_mouse_event(INPUT_LBUTTON_DOWN, mouse_x, mouse_y,
												MOUSE_LBUTTON, MWHEEL_STATIONARY);

			case Button3:
				// Right mouse button
				key_pressed_frames[MOUSE_RBUTTON] = frame;
				key_released_frames[MOUSE_RBUTTON] = 0;

				return input_handle_mouse_event(INPUT_RBUTTON_DOWN, mouse_x, mouse_y,
												MOUSE_RBUTTON, MWHEEL_STATIONARY);
				
			case Button2:
				// Middle mouse button (wheel)
				key_pressed_frames[MOUSE_MBUTTON] = frame;
				key_released_frames[MOUSE_MBUTTON] = 0;

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
				key_pressed_frames[MOUSE_LBUTTON] = 0;
				key_released_frames[MOUSE_LBUTTON] = frame;

				XUngrabPointer(button_event->display, CurrentTime);

				return input_handle_mouse_event(INPUT_LBUTTON_UP, mouse_x, mouse_y,
												MOUSE_LBUTTON, MWHEEL_STATIONARY);

			case Button3:
				// Right mouse button
				key_pressed_frames[MOUSE_RBUTTON] = 0;
				key_released_frames[MOUSE_RBUTTON] = frame;

				return input_handle_mouse_event(INPUT_RBUTTON_UP, mouse_x, mouse_y,
										 MOUSE_RBUTTON, MWHEEL_STATIONARY);
			case Button2:
				// Middle mouse button (wheel)
				key_pressed_frames[MOUSE_MBUTTON] = 0;
				key_released_frames[MOUSE_MBUTTON] = frame;

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

void input_sys_warp_cursor(int16_t x, int16_t y)
{
	Display *display = window_get_display();
	Window root = XRootWindow(display, 0);

	XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
}

uint32_t input_sys_get_key_pressed_frame(uint32_t key_symbol)
{
	// Symbols below 8 are reserved for mouse buttons.
	if (key_symbol < 8) {
		return key_pressed_frames[key_symbol];
	}

	// Map the key symbol to a keycode (a value between 8 and 255).
	KeyCode code = XKeysymToKeycode(window_get_display(), key_symbol);

	if (code != 0) {
		return key_pressed_frames[code];
	}

	return 0;
}

uint32_t input_sys_get_key_released_frame(uint32_t key_symbol)
{
	if (key_symbol < 8) {
		return key_released_frames[key_symbol];
	}

	KeyCode code = XKeysymToKeycode(window_get_display(), key_symbol);

	if (code != 0) {
		return key_released_frames[code];
	}

	return 0;
}
