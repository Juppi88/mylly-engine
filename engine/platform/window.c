#include "window.h"

#ifdef _WIN32

#include <Windows.h>

#error Missing WIN32 implementation for window.c

#else

#include "io/log.h"
#include "input/input.h"
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

// --------------------------------------------------------------------------------

static Display *display;
static XVisualInfo *visual;
static Window window;
static Window root;

// --------------------------------------------------------------------------------

bool window_create(bool fullscreen, int width, int height)
{
	// Already connected to the X server, don't do anything.
	if (display != NULL) {
		return false;
	}

	// Connect to the X server.
	display = XOpenDisplay(NULL);

	if (display == NULL) {
		return false;
	}

	// Get the root window.
	root = DefaultRootWindow(display);

	// Prepare settings for an OpenGL rendering context.
	GLint attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	visual = glXChooseVisual(display, 0, attr);

	if (visual == NULL) {
		XCloseDisplay(display);
		return false;
	}

	// Set essential window attributes before creating the main window.
	Colormap colorMap = XCreateColormap(display, root, visual->visual, AllocNone);

	XSetWindowAttributes windowAttr;
	windowAttr.colormap = colorMap;
	windowAttr.event_mask = ExposureMask | KeyPressMask;

	// And finally, create the window.
	int x = 0, y = 0;

	window = XCreateWindow(display, root, x, y, width, height, 0, visual->depth, InputOutput,
						   visual->visual, CWColormap | CWEventMask, &windowAttr);

	XMapWindow(display, window);
	XStoreName(display, window, "Mylly");

	XClassHint hint = { "Mylly", "Mylly" };
	XSetClassHint(display, window, &hint);

	// Request input events from the X server for input processing and key binds.
	XSelectInput(display, window, KeyPressMask | KeyReleaseMask | PointerMotionMask |
								  ButtonPressMask | ButtonReleaseMask);

	// Move cursor to the center of the new window.
	input_set_cursor_position(width / 2, height / 2);

	log_message("Platform", "Main window was created successfully, handle: 0x%X", (void *)window);

	return true;
}

void window_pump_events(void)
{
	XWindowAttributes xwa;
	XExposeEvent event;

	XGetWindowAttributes(display, window, &xwa);

	event.type = Expose;
	event.serial = 0;
	event.send_event = true;
	event.display = display;
	event.window = window;
	event.x = 0;
	event.y = 0;
	event.width = xwa.width;
	event.height = xwa.height;
	event.count = 0;

	XSendEvent(display, window, 0, ExposureMask, (XEvent *)&event);
}

Display *window_get_display(void)
{
	return display;
}

Window window_get_handle(void)
{
	return window;
}

XVisualInfo *window_get_visual_info(void)
{
	return visual;
}

void window_process_events(input_hook_t handler)
{
	XEvent event;

	while (XPending(display)) {

		XNextEvent(display, &event);

		if (handler != NULL) {
			handler(&event);
		}
	}
}

#endif
