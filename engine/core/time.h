#pragma once
#ifndef __TIME_H
#define __TIME_H

#include "core/defines.h"
#include "math/vector.h"

// --------------------------------------------------------------------------------

typedef struct engine_time_t {

	float time; // Engine time since startup [s]
	float delta_time; // Time since last frame [s]
	uint32_t frame_count; // Number of frames rendered

} engine_time_t;

// --------------------------------------------------------------------------------

void time_initialize(void);
void time_tick(void);

// --------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

static INLINE engine_time_t get_time(void)
{
	extern engine_time_t engine_time;
	return engine_time;
}

static INLINE vec4_t get_shader_time(void)
{
	extern vec4_t engine_shader_time;
	return engine_shader_time;
}

END_DECLARATIONS;

#endif
