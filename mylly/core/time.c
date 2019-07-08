#include "core/time.h"
#include "platform/timer.h"
#include "math/math.h"

// --------------------------------------------------------------------------------

// Time, DeltaTime, FrameCount
engine_time_t engine_time = { 0, 0, 0, 0, 1, 1 };

// Time, CosTime, SinTime, DeltaTime
vec4_t engine_shader_time = { .vec = { 0, 1, 0, 0 } };

// The tick count for when the engine is initialized.
static uint64_t start = 0;
static uint64_t previous = 0;

// --------------------------------------------------------------------------------

void time_initialize(void)
{
	start = timer_get_ticks();
	previous = start;

	// Set initial delta time to a non-zero value to avoid division by zero.
	engine_time.delta_time = 1 / 60.0f;
	engine_time.real_delta_time = 1 / 60.0f;
	engine_shader_time.w = 1 / 60.0f;
}

void time_tick(void)
{
	// TODO: Use high precision clock!
	uint64_t now = timer_get_ticks();
	uint64_t elapsed = now - start;

	float real_time = 0.001f * elapsed;
	float delta = 0.001f * (now - previous);
	float real_delta = 0.001f * (now - previous);

	// Update engine time.
	engine_time.delta_time = engine_time.scale * delta;
	engine_time.real_delta_time = real_delta;
	engine_time.time += engine_time.delta_time;
	engine_time.real_time = real_time;
	engine_time.frame_count++;
	
	// Update shader time.
	engine_shader_time.x = engine_time.time;
	engine_shader_time.w = engine_time.delta_time;

	math_sincos(engine_time.time, &engine_shader_time.z, &engine_shader_time.y);

	previous = now;
}

void time_set_scale(float scale)
{
	if (scale < 0) {
		return;
	}

	engine_time.scale = scale;
}
