#pragma once
#ifndef __DEBUG_H
#define __DEBUG_H

#include "core/defines.h"
#include "math/vector.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void debug_initialize(void);
void debug_shutdown(void);

void debug_begin_frame(void);
void debug_end_frame(void);

void debug_draw_line(vec3_t start, vec3_t end, colour_t colour);
void debug_draw_2d_line(vec2_t start, vec2_t end, colour_t colour);
void debug_draw_2d_rect(vec2_t min, vec2_t max, colour_t colour);

END_DECLARATIONS;

#endif
