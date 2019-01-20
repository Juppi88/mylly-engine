#pragma once
#ifndef __MATH_H
#define __MATH_H

#include "core/defines.h"
#include "math/vector.h"
#include "renderer/colour.h"
#include <math.h>

// -------------------------------------------------------------------------------------------------

#ifdef PI
#undef PI
#endif

#define PI 3.1415926535f
#define FLOAT_EPSILON 1.192092896e-07f

#define RAD_TO_DEG(x) ((x) * 180.0f / PI)
#define DEG_TO_RAD(x) ((x) * PI / 180.0f)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ABS(a) ((a) < 0 ? -(a) : (a))

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define SIGN(x) ((x) < 0 ? -1 : 1)

#define IS_ZERO(x) (fabs(x) < 0.00000001f)

// -------------------------------------------------------------------------------------------------

static INLINE void math_sincos(float angle, float *s, float *c)
{
	*s = sinf(angle);
	*c = cosf(angle);
}

static INLINE float math_sqrt(float value)
{
	return sqrtf(value);
}

static INLINE float math_sanitize_angle(float angle)
{
	while (angle > 2 * PI) { angle -= 2 * PI; }
	while (angle < 0) { angle += 2 * PI; }
	return angle;
}

// -------------------------------------------------------------------------------------------------

static INLINE int lerpi(int a, int b, float t)
{
	if (a == b) {
		return a; 
	}
	
	t = CLAMP(t, 0, 1);
	return a + (int)(t * (b - a));
}

static INLINE float lerpf(float a, float b, float t)
{
	t = CLAMP(t, 0, 1);
	return a + t * (b - a);
}

static INLINE vec3_t lerpv(const vec3_t a, const vec3_t b, float t)
{
	return vec3(
		lerpf(a.x, b.x, t),
		lerpf(a.y, b.y, t),
		lerpf(a.z, b.z, t)
	);
}

static INLINE colour_t lerpc(const colour_t a, const colour_t b, float t)
{
	return col_a(
		(uint8_t)lerpi(a.r, b.r, t),
		(uint8_t)lerpi(a.g, b.g, t),
		(uint8_t)lerpi(a.b, b.b, t),
		(uint8_t)lerpi(a.a, b.a, t)
	);
}

#endif
