#include "core/time.h"
#include "platform/timer.h"
#include "math/math.h"

// --------------------------------------------------------------------------------

// Time, DeltaTime, FrameCount
engine_time_t engine_time = { 0, 0, 1 };

// Time, CosTime, SinTime, DeltaTime
vec4_t engine_shader_time = { .vec = { 0, 1, 0, 0 } };

// The tick count for when the engine is initialized.
static uint64_t start;

// --------------------------------------------------------------------------------

void time_initialize(void)
{
	start = timer_get_ticks();

	// Set initial delta time to a non-zero value to avoid division by zero.
	engine_time.delta_time = 1 / 60.0f;
	engine_shader_time.w = 1 / 60.0f;
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
