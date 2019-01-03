#include "window.h"
#include "io/log.h"
#include "io/input.h"

#ifdef _WIN32

// -------------------------------------------------------------------------------------------------

#include <Windows.h>
#include "core/mylly.h"

// -------------------------------------------------------------------------------------------------

static HWND window_handle;

// -------------------------------------------------------------------------------------------------

bool window_create(bool fullscreen, int monitor, int x, int y, int width, int height)
{
	if (window_handle != NULL) {
		return true;
	}

	HINSTANCE instance = GetModuleHandle(NULL);

	// Register the window class.
	WNDCLASS window_class;
	memset(&window_class, 0, sizeof(window_class));

	window_class.style = 0;
	window_class.lpfnWndProc = NULL;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.lpszMenuName = 0;
	window_class.lpszClassName = "Mylly";

	if (!RegisterClass(&window_class)) {
		return false;
	}

	// Alter window style depending on whether the window should be fullscreen or not.
	int	ext_style, style;

	if (fullscreen) {
		ext_style = WS_EX_TOPMOST;
		style = (WS_POPUP | WS_VISIBLE | WS_SYSMENU);
	}
	else {
		ext_style = 0;
		style = (WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE | WS_SYSMENU);
	}

	// Create the window.
	window_handle = CreateWindowEx(
		ext_style, "Mylly", "Mylly", style,
		x, y, width, height,
		NULL, NULL, instance, NULL);

	if (window_handle == NULL) {
		return false;
	}

	// Window creation succeeded! Show it to the world.
	ShowWindow(window_handle, SW_SHOW);
	UpdateWindow(window_handle);
	SetForegroundWindow(window_handle);
	SetFocus(window_handle);

	// Move cursor to the center of the new window.
	input_set_cursor_position(width / 2, height / 2);

	log_message("Platform", "Main window was created successfully, resolution: %ux%u",
		width, height);

	return true;
}

void window_pump_events(void)
{
	// There's nothing we have to do here on Windows.
	// Events are read and processed in window_process_events.
	return;
}

void window_process_events(input_hook_t handler)
{
	MSG msg;

	// Read all queued messages.
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {

		if (GetMessage(&msg, NULL, 0, 0) <= 0) {
			
			// WM_QUIT (or an error) received, request engine shutdown.
			log_message("Platform", "Main window was closed, shutting down.");
			mylly_exit();
		}

		// Invoke the engine's event hook.
		if (handler != NULL) {
			handler(&msg);
		}

		// Dispatch the message to the default handler.
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool window_get_monitor_info(int monitor_id, monitor_info_t *info_dest)
{
	if (window_handle == NULL) {
		return false;
	}

	// Get the monitor the engine window is on.
	HMONITOR monitor = MonitorFromWindow(window_handle, MONITOR_DEFAULTTONEAREST);

	// Retrieve information about the monitor.
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);

	if (!GetMonitorInfo(monitor, &info)) {
		return false;
	}

	if (info_dest != NULL) {

		info_dest->x = info.rcMonitor.right;
		info_dest->y = info.rcMonitor.top;
		info_dest->width = info.rcMonitor.right - info.rcMonitor.left;
		info_dest->height = info.rcMonitor.bottom - info.rcMonitor.top;
	}

	return true;
}

HWND window_get_handle(void)
{
	return window_handle;
}

#else

// -------------------------------------------------------------------------------------------------

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <GL/gl.h>
#include <GL/glx.h>

// -------------------------------------------------------------------------------------------------

static Display *display;
static XVisualInfo *visual;
static Window window;
static Window root;

// -------------------------------------------------------------------------------------------------

bool window_create(bool fullscreen, int monitor, int x, int y, int width, int height)
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
	window = XCreateWindow(display, root, x, y, width, height, 0, visual->depth, InputOutput,
						   visual->visual, CWColormap | CWEventMask, &windowAttr);

	XMapWindow(display, window);
	XStoreName(display, window, "Mylly");

	XClassHint hint = { "Mylly", "Mylly" };
	XSetClassHint(display, window, &hint);

	// Request input events from the X server for input processing and key binds.
	XSelectInput(display, window, KeyPressMask | KeyReleaseMask | PointerMotionMask |
								  ButtonPressMask | ButtonReleaseMask);

	// Force the position on the screen. This is in case the window manager decides to move the
	// window to someplace else.
	XWindowChanges xwc;
	xwc.x = x;
	xwc.y = y;

	monitor_info_t monitor_info;

	if (window_get_monitor_info(monitor, &monitor_info)) {
		xwc.x += monitor_info.x;
		xwc.y += monitor_info.y;
	}

	XConfigureWindow(display, window, CWX | CWY, &xwc);

	// Move cursor to the center of the new window.
	input_set_cursor_position(width / 2, height / 2);

	log_message("Platform", "Main window was created successfully, resolution: %ux%u",
		width, height);

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

bool window_get_monitor_info(int monitor, monitor_info_t *info_dest)
{
	Display *display = XOpenDisplay(NULL);

	if (display == NULL) {
		return false;
	}

	Window root = DefaultRootWindow(display);

	// Get screen resources.
	XRRScreenResources *screen_info = XRRGetScreenResources(display, root);

	// Ensure the screen exists.
	if (monitor < 0 || monitor >= screen_info->noutput) {
		return false;
	}

	// Get monitor information.
	XRRCrtcInfo *monitor_info = XRRGetCrtcInfo(display, screen_info, screen_info->crtcs[monitor]);

	if (info_dest != NULL) {
		info_dest->x = monitor_info->x;
		info_dest->y = monitor_info->y;
		info_dest->width = monitor_info->width;
		info_dest->height = monitor_info->height;
	}

	return true;
}

#endif
