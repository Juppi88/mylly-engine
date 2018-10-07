#pragma once
#ifndef __PLATFORM_INPUT_H
#define __PLATFORM_INPUT_H

#include "core/defines.h"

bool input_process_messages(void *params);
void input_warp_cursor(int16_t x, int16_t y);

#endif
