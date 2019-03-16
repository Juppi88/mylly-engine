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
void debug_process_drawings(bool overlay);

void debug_draw_line(vec3_t start, vec3_t end, colour_t colour, bool overlay);
void debug_draw_line_2d(vec2_t start, vec2_t end, colour_t colour, bool overlay);
void debug_draw_rect(vec2_t min, vec2_t max, colour_t colour, bool overlay);

END_DECLARATIONS;

#endif
