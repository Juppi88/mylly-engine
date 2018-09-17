#pragma once
#ifndef __QUATERNION_H
#define __QUATERNION_H

#include "vector.h"

// --------------------------------------------------------------------------------
// quat_t

typedef struct quat_t {
	float x, y, z, w;
} quat_t;

#define quat(x, y, z, w) { x, y, z, w }
#define quat_identity() { 0, 0, 0, 1 }

extern quat_t quat_identity;

// --------------------------------------------------------------------------------

quat_t quat_from_euler(float x, float y, float z);
quat_t quat_from_euler3(const vec3_t *euler);

// --------------------------------------------------------------------------------

#endif
