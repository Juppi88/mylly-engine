#pragma once
#ifndef __QUATERNION_H
#define __QUATERNION_H

#include "vector.h"

BEGIN_DECLARATIONS;

// --------------------------------------------------------------------------------
// quat_t

typedef struct quat_t {
	float x, y, z, w;
} quat_t;

#define quat(x, y, z, w) { x, y, z, w }
#define quat_identity() { 0, 0, 0, 1 }

extern quat_t quat_identity;

// --------------------------------------------------------------------------------

static INLINE quat_t quaternion(float x, float y, float z, float w)
{
	quat_t q = quat(x, y, z, w);
	return q;
}

quat_t quat_from_euler(float x, float y, float z);
quat_t quat_from_euler3(const vec3_t *euler);
vec3_t quat_to_euler(const quat_t *quat);

quat_t quat_multiply(const quat_t *a, const quat_t *b);
vec3_t quat_multiply_vec3(const quat_t *q, const vec3_t *v);

// --------------------------------------------------------------------------------

static INLINE void quat_print(const quat_t *quat)
{
	printf("%.2f  %.2f  %.2f  %.2f\n", quat->x, quat->y, quat->z, quat->w);
}

// --------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
