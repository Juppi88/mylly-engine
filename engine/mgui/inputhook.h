#pragma once
#ifndef __MGUI_INPUTHOOK_H
#define __MGUI_INPUTHOOK_H

#include "io/input.h"

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

bool mgui_handle_keyboard_event(input_event_t type, uint32_t key);

bool mgui_handle_mouse_event(input_event_t type, int16_t x, int16_t y,
							 mouse_button_t button, mouse_wheel_t wheel);

END_DECLARATIONS;

#endif
