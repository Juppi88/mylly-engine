#pragma once
#ifndef __WINDOW_H
#define __WINDOW_H

#include "core/defines.h"

typedef bool (*input_hook_t)(void *args);

// --------------------------------------------------------------------------------

bool window_create(bool fullscreen, int width, int height);
void window_pump_events(void);
void window_process_events(input_hook_t handler);

#ifndef _WIN32

// X specific methods for the renderer.
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

Display *window_get_display(void);
Window window_get_handle(void);
XVisualInfo *window_get_visual_info(void);

#endif

#endif
