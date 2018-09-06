#pragma once
#ifndef __MATH_H
#define __MATH_H

#include "core/defines.h"
#include <math.h>

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
