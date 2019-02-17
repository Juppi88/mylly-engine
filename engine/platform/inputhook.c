#include "inputhook.h"
#include "window.h"
#include "io/input.h"
#include "core/time.h"

// -------------------------------------------------------------------------------------------------

#define KEY_CODES 256 // Number of key codes that a keyboard can have

// Modifier keys being pressed.
static uint32_t modifier_flags = 0;

// An array containing the specific frame count when a key is pressed down or released.
// If a key is currently not pressed down, the frame is set to 0.
// Values < 8 are reserved for mouse buttons.
static uint32_t key_pressed_frames[KEY_CODES];
static uint32_t key_released_frames[KEY_CODES];

// -------------------------------------------------------------------------------------------------

#ifdef _WIN32

#include <Windows.h>
#include <Windowsx.h>

bool input_sys_process_messages(void *params)
{
	uint32_t frame = get_time().frame_count;
	uint32_t keycode = 0;
	int16_t mouse_x, mouse_y;
	MSG *msg = (MSG *)params;

	switch (msg->message) {

	case WM_KEYDOWN:
		keycode = (uint32_t)msg->wParam;

		if (keycode >= KEY_CODES) {
			return true;
		}

		// Store the frame the button is pressed.
		if (key_pressed_frames[keycode] == 0) {

			// Update the frame only when the key is pressed down for the first time.
			// Other KeyPress events are just character repeats.
			key_pressed_frames[keycode] = frame;
			key_released_frames[keycode] = 0;
		}

		// Handle key down event and related binds.
		return input_handle_keyboard_event(INPUT_KEY_DOWN, keycode);

	case WM_KEYUP:
		keycode = (uint32_t)msg->wParam;

		if (keycode >= KEY_CODES) {
			return true;
		}

		// Key is released, update frame counts.
		key_pressed_frames[keycode] = 0;
		key_released_frames[keycode] = frame;

		// Handle key up event and related binds.
		return input_handle_keyboard_event(INPUT_KEY_UP, keycode);

	case WM_CHAR:
		keycode = (uint32_t)msg->wParam;
		return input_handle_keyboard_event(INPUT_CHARACTER, keycode);

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:

		switch (msg->message) {
			case WM_LBUTTONDOWN: keycode = MOUSE_LEFT; break;
			case WM_RBUTTONDOWN: keycode = MOUSE_RIGHT; break;
			case WM_MBUTTONDOWN: keycode = MOUSE_MIDDLE; break;
		}

		key_pressed_frames[keycode] = frame;
		key_released_frames[keycode] = 0;

		mouse_x = GET_X_LPARAM(msg->lParam);
		mouse_y = GET_Y_LPARAM(msg->lParam);

		// Handle mouse button event and related binds.
		return input_handle_mouse_event(INPUT_MOUSE_BUTTON_DOWN, mouse_x, mouse_y,
			keycode, MWHEEL_STATIONARY);

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:

		switch (msg->message) {
			case WM_LBUTTONUP: keycode = MOUSE_LEFT; break;
			case WM_RBUTTONUP: keycode = MOUSE_RIGHT; break;
			case WM_MBUTTONUP: keycode = MOUSE_MIDDLE; break;
		}
		
		key_pressed_frames[keycode] = 0;
		key_released_frames[keycode] = frame;

		mouse_x = GET_X_LPARAM(msg->lParam);
		mouse_y = GET_Y_LPARAM(msg->lParam);

		// Handle mouse button event and related binds.
		return input_handle_mouse_event(INPUT_MOUSE_BUTTON_UP, mouse_x, mouse_y,
			keycode, MWHEEL_STATIONARY);

	case WM_MOUSEMOVE:

		mouse_x = GET_X_LPARAM(msg->lParam);
		mouse_y = GET_Y_LPARAM(msg->lParam);

		return input_handle_mouse_event(INPUT_MOUSE_MOVE, mouse_x, mouse_y,
			MOUSE_NONE, MWHEEL_STATIONARY);

	case WM_MOUSEWHEEL:

		mouse_x = GET_X_LPARAM(msg->lParam);
		mouse_y = GET_Y_LPARAM(msg->lParam);
		keycode = GET_WHEEL_DELTA_WPARAM(msg->wParam) > 0 ? MWHEEL_UP : MWHEEL_DOWN;

		return input_handle_mouse_event(INPUT_MOUSE_WHEEL, mouse_x, mouse_y,
			MOUSE_NONE, keycode);
	}

	return true;
}

void input_sys_warp_cursor(int16_t x, int16_t y)
{
	POINT point = { x, y };

	ClientToScreen(window_get_handle(), &point);
	SetCursorPos(point.x, point.y);
}

uint32_t input_sys_get_key_pressed_frame(uint32_t key_symbol)
{
	if (key_symbol < KEY_CODES) {
		return key_pressed_frames[key_symbol];
	}

	return 0;
}

uint32_t input_sys_get_key_released_frame(uint32_t key_symbol)
{
	if (key_symbol < KEY_CODES) {
		return key_released_frames[key_symbol];
	}

	return 0;
}

#else

// -------------------------------------------------------------------------------------------------

#include <X11/Xlib.h>
#include <X11/Xutil.h>

// -------------------------------------------------------------------------------------------------

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
			if (key_pressed_frames[key_event->keycode] == 0) {

				// Update the frame only when the key is pressed down for the first time.
				// Other KeyPress events are just character repeats.
				key_pressed_frames[key_event->keycode] = frame;
				key_released_frames[key_event->keycode] = 0;
			}

			// Store modifier flags.
			modifier_flags = key_event->state;

			// Get the symbol (key code) for the pressed button.
			XLookupString(key_event, buffer, sizeof(buffer), &sym, NULL);
			code = (uint32_t)sym;

			// A fix to make Windows and Linux hooks compatible:
			// Convert lowercase characters to upper case before processing hooks.
			if (code >= 'a' && code <= 'z') {
				code -= ('a' - 'A');
			}

			// Handle key down event and related binds.
			if (!input_handle_keyboard_event(INPUT_KEY_DOWN, code)) {
				return false;
			}

			// Handle character event and related bindings.
			if (*buffer != 0) {
				return input_handle_keyboard_event(INPUT_CHARACTER, *buffer);
			}
			return true;
		}

		case KeyRelease: {
			key_event = (XKeyEvent *)event;

			// Look up the next event. If it is a KeyPress event for the same key, ignore the
			// KeyRelease event. This is to ignore X11's BS autorepeat feature.
			if (XEventsQueued(key_event->display, QueuedAfterReading)) {

				XEvent next;
				XPeekEvent(key_event->display, &next);

				if (next.type == KeyPress && next.xkey.time == event->xkey.time &&
                    next.xkey.keycode == event->xkey.keycode) {
					break;
				}
			}

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
				key_pressed_frames[MOUSE_LEFT] = frame;
				key_released_frames[MOUSE_LEFT] = 0;

				XGrabPointer(button_event->display, button_event->window, false,
							 ButtonPressMask|ButtonReleaseMask|PointerMotionMask|FocusChangeMask|
							 EnterWindowMask|LeaveWindowMask,
							 GrabModeAsync, GrabModeAsync, button_event->window, None, CurrentTime);

				// Handle mouse button event and related binds.
				return input_handle_mouse_event(INPUT_MOUSE_BUTTON_DOWN, mouse_x, mouse_y,
												MOUSE_LEFT, MWHEEL_STATIONARY);

			case Button3:
				// Right mouse button
				key_pressed_frames[MOUSE_RIGHT] = frame;
				key_released_frames[MOUSE_RIGHT] = 0;

				return input_handle_mouse_event(INPUT_MOUSE_BUTTON_DOWN, mouse_x, mouse_y,
												MOUSE_RIGHT, MWHEEL_STATIONARY);
				
			case Button2:
				// Middle mouse button (wheel)
				key_pressed_frames[MOUSE_MIDDLE] = frame;
				key_released_frames[MOUSE_MIDDLE] = 0;

				return input_handle_mouse_event(INPUT_MOUSE_BUTTON_DOWN, mouse_x, mouse_y,
												MOUSE_MIDDLE, MWHEEL_STATIONARY);
				
			case Button4:
				// Mouse wheel scroll up
				return input_handle_mouse_event(INPUT_MOUSE_WHEEL, mouse_x, mouse_y,
												MOUSE_NONE, MWHEEL_UP);

			case Button5:
				// Mouse wheel scroll down
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
				key_pressed_frames[MOUSE_LEFT] = 0;
				key_released_frames[MOUSE_LEFT] = frame;

				XUngrabPointer(button_event->display, CurrentTime);

				return input_handle_mouse_event(INPUT_MOUSE_BUTTON_UP, mouse_x, mouse_y,
												MOUSE_LEFT, MWHEEL_STATIONARY);

			case Button3:
				// Right mouse button
				key_pressed_frames[MOUSE_RIGHT] = 0;
				key_released_frames[MOUSE_RIGHT] = frame;

				return input_handle_mouse_event(INPUT_MOUSE_BUTTON_UP, mouse_x, mouse_y,
										 MOUSE_RIGHT, MWHEEL_STATIONARY);
			case Button2:
				// Middle mouse button (wheel)
				key_pressed_frames[MOUSE_MIDDLE] = 0;
				key_released_frames[MOUSE_MIDDLE] = frame;

				return input_handle_mouse_event(INPUT_MOUSE_BUTTON_UP, mouse_x, mouse_y,
												MOUSE_MIDDLE, MWHEEL_STATIONARY);
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
	Window window = window_get_handle();
	Window root = XRootWindow(display, 0);

	// Translate the coordinates and find the target window to be used.
	int target_x, target_y;
	Window child;

	XTranslateCoordinates(display, window,root, x, y, &target_x, &target_y, &child);

	// Do the warping.
	XWarpPointer(display, root, child, 0, 0, 0, 0, target_x, target_y);
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

#endif
