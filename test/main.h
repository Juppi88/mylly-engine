#pragma once
#ifndef __MAIN_H
#define __MAIN_H

#include "minunit.h"
#include "core/defines.h"
#include "math/quaternion.h"
#include "math/math.h"

#define EPSILON 0.01f

static INLINE int quat_equals(const quat_t a, const quat_t b)
{
	if (fabsf(a.x - b.x) > EPSILON) return 0;
	if (fabsf(a.y - b.y) > EPSILON) return 0;
	if (fabsf(a.z - b.z) > EPSILON) return 0;
	if (fabsf(a.w - b.w) > EPSILON) return 0;
	return 1;
}

static INLINE int vec3_equals(const vec3_t a, const vec3_t b)
{
	if (fabsf(a.x - b.x) > EPSILON) return 0;
	if (fabsf(a.y - b.y) > EPSILON) return 0;
	if (fabsf(a.z - b.z) > EPSILON) return 0;
	return 1;
}

static INLINE void quat_print2(const quat_t q)
{
	printf("%ff, %ff, %ff, %ff\n", q.x, q.y, q.z, q.w);
}

static INLINE void vec3_print2(const vec3_t v)
{
	printf("%ff, %ff, %ff\n", v.x, v.y, v.z);
}

#endif
