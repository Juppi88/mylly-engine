#pragma once
#ifndef __WINDOW_H
#define __WINDOW_H

#include "core/defines.h"

bool window_create(bool fullscreen, int width, int height);
void window_pump_events(void);

#endif
