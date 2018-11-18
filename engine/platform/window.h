#pragma once
#ifndef __WINDOW_H
#define __WINDOW_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef bool (*input_hook_t)(void *args);

typedef struct monitor_info_t {
	int x, y; // Position of the monitor
	int width, height; // Resolution of the monitor
} monitor_info_t;

// -------------------------------------------------------------------------------------------------

bool window_create(bool fullscreen, int monitor, int x, int y, int width, int height);
void window_pump_events(void);
void window_process_events(input_hook_t handler);

bool window_get_monitor_info(int monitor, monitor_info_t *info_dest);

#ifndef _WIN32

// X specific methods for the renderer.
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

Display *window_get_display(void);
Window window_get_handle(void);
XVisualInfo *window_get_visual_info(void);

#endif

END_DECLARATIONS;

#endif
