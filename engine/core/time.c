#include "core/time.h"
#include "platform/timer.h"
#include "math/math.h"

// --------------------------------------------------------------------------------

engine_time_t engine_time;
vec4_t engine_shader_time = vec4(0, 1, 0, 0); // Time, CosTime, SinTime, DeltaTime

static uint64_t start;

// --------------------------------------------------------------------------------

void time_initialize(void)
{
	start = timer_get_ticks();
}

void time_tick(void)
{
	// TODO: Use high precision clock!
	uint64_t now = timer_get_ticks();
	uint64_t elapsed = now - start;

	float time = 0.001f * elapsed;

	// Update engine time.
	engine_time.delta_time = (time - engine_time.time);
	engine_time.time = time;
	engine_time.frame_count++;

	// Update shader time.
	engine_shader_time.x = time;
	engine_shader_time.w = engine_time.delta_time;

	math_sincos(time, &engine_shader_time.z, &engine_shader_time.y);
}
