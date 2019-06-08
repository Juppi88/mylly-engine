#pragma once
#ifndef __RANDOM_H
#define __RANDOM_H

#include "core/defines.h"
#include "math/math.h"
#include "renderer/colour.h"
#include <stdlib.h>

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

static INLINE int randomi(int a, int b)
{
	if (a == b) {
		return a; 
	}

	return a + rand() % (b - a);
}

static INLINE float randomf(float a, float b)
{
	return a + ((double)rand() / (RAND_MAX)) * (b - a);
}

static INLINE vec3_t randomv(const vec3_t a, const vec3_t b)
{
	return vec3(
		randomf(a.x, b.x),
		randomf(a.y, b.y),
		randomf(a.z, b.z)
	);
}

static INLINE colour_t randomc(const colour_t a, const colour_t b)
{
	float t = randomf(0, 1);

	return col_a(
		(uint8_t)lerpi(a.r, b.r, t),
		(uint8_t)lerpi(a.g, b.g, t),
		(uint8_t)lerpi(a.b, b.b, t),
		(uint8_t)lerpi(a.a, b.a, t)
	);
}

vec3_t random_point_on_shpere(void);
vec3_t random_point_on_cone(float angle);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
