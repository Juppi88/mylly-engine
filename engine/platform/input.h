#pragma once
#ifndef __PLATFORM_INPUT_H
#define __PLATFORM_INPUT_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

// Process input related window messages and relay them to the input library.
bool input_sys_process_messages(void *params);

// Warp the mouse cursor to a specific position on the monitor.
void input_sys_warp_cursor(int16_t x, int16_t y);

// Returns the frame count when a specific key was pressed down, 0 if the key is not pressed.
uint32_t input_sys_get_key_pressed_frame(uint32_t key_symbol);

// Returns the frame count when a specific key was released.
uint32_t input_sys_get_key_released_frame(uint32_t key_symbol);

END_DECLARATIONS;

#endif
