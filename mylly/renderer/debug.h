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
void debug_end_frame(camera_t *camera);

void debug_draw_line(vec3_t start, vec3_t end, colour_t colour, bool overlay);
void debug_draw_rect(vec3_t centre, vec2_t extents, colour_t colour, bool overlay);
void debug_draw_circle(vec3_t origin, float radius, colour_t colour, bool overlay);

END_DECLARATIONS;

#endif
