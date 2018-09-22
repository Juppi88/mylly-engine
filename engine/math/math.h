#pragma once
#ifndef __MATH_H
#define __MATH_H

#include "core/defines.h"
#include <math.h>

#ifdef PI
#undef PI
#endif

#define PI 3.1415926535f
#define RAD_TO_DEG(x) ((x) * 180.0f / PI)
#define DEG_TO_RAD(x) ((x) * PI / 180.0f)

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

#endif
