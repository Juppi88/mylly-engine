#pragma once
#ifndef __RENDERER_H
#define __RENDERER_H

#include "core/defines.h"

bool rend_initialize(void);
void rend_shutdown(void);

void rend_draw_view(void);

#endif
