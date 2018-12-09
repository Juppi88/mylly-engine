#pragma once
#ifndef __MATH_H
#define __MATH_H

#include "core/defines.h"
#include <math.h>

#ifdef PI
#undef PI
#endif

#define PI 3.1415926535f
#define FLOAT_EPSILON 1.192092896e-07f

#define RAD_TO_DEG(x) ((x) * 180.0f / PI)
#define DEG_TO_RAD(x) ((x) * PI / 180.0f)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define SIGN(x) ((x) < 0 ? -1 : 1)

#define IS_ZERO(x) (fabs(x) < 0.00000001f)

// --------------------------------------------------------------------------------

static INLINE void math_sincos(float angle, float *s, float *c)
{
	*s = sinf(angle);
	*c = cosf(angle);
}

static INLINE float math_sqrt(float value)
{
	return sqrt(value);
}

static INLINE float math_sanitize_angle(float angle)
{
	while (angle > 2 * PI) { angle -= 2 * PI; }
	while (angle < 0) { angle += 2 * PI; }
	return angle;
}

#endif
