#pragma once
#ifndef __QUATERNION_H
#define __QUATERNION_H

#include "vector.h"

BEGIN_DECLARATIONS;

// --------------------------------------------------------------------------------
// quat_t

typedef vec4_t quat_t;

#if !defined(__cplusplus) || !defined(_MSC_VER)
	#define quat(x, y, z, w) (quat_t){ .vec = { x, y, z, w } }
	#define quat_identity() (quat_t){ .vec = { 0, 0, 0, 1 } }
#else
	#define quat(x, y, z, w) { x, y, z, w }
	#define quat_identity() { 0, 0, 0, 1 }
#endif

// --------------------------------------------------------------------------------

static INLINE quat_t quaternion(float x, float y, float z, float w)
{
	quat_t q = quat(x, y, z, w);
	return q;
}

quat_t quat_from_euler(float x, float y, float z);
quat_t quat_from_euler_deg(float x, float y, float z);
vec3_t quat_to_euler(quat_t quat);

quat_t quat_multiply(quat_t a, quat_t b);
vec3_t quat_rotate_vec3(quat_t q, vec3_t v);

quat_t quat_inverse(quat_t q);

// --------------------------------------------------------------------------------

static INLINE void quat_print(quat_t quat)
{
	printf("%.2f  %.2f  %.2f  %.2f\n", quat.x, quat.y, quat.z, quat.w);
}

// --------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
