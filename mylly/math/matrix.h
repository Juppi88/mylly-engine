#pragma once
#ifndef __MATRIX_H
#define __MATRIX_H

#include "math/vector.h"
#include "math/quaternion.h"
#include "core/defines.h"
#include <stdio.h>
#include <string.h>

BEGIN_DECLARATIONS;

// --------------------------------------------------------------------------------
// mat_t - A 4x4 matrix structure

typedef struct mat_t {
	float col[4][4]; // Column vectors
} mat_t;

#if !defined(__cplusplus) || !defined(_MSC_VER)
	#define mat_zero() (mat_t){ { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } }
	#define mat_identity() (mat_t){ { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } } }
#else
	#define mat_zero() { { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } }
	#define mat_identity() { { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } } }
#endif

#define mat_as_ptr(mat) &(mat).col[0][0]

// --------------------------------------------------------------------------------

vec3_t mat_multiply3(mat_t mat, vec3_t v);
vec4_t mat_multiply4(mat_t mat, vec4_t v);
void mat_multiply(mat_t mat1, mat_t mat2, mat_t *out);
quat_t mat_to_quat(mat_t mat);

mat_t mat_invert(mat_t mat);

// --------------------------------------------------------------------------------

static INLINE void mat_set(
	mat_t *mat,
	float m11, float m12, float m13, float m14, // Column 1
	float m21, float m22, float m23, float m24, // Column 2
	float m31, float m32, float m33, float m34, // Column 3
	float m41, float m42, float m43, float m44) // Column 4
{
	mat->col[0][0] = m11;
	mat->col[0][1] = m12;
	mat->col[0][2] = m13;
	mat->col[0][3] = m14;

	mat->col[1][0] = m21;
	mat->col[1][1] = m22;
	mat->col[1][2] = m23;
	mat->col[1][3] = m24;

	mat->col[2][0] = m31;
	mat->col[2][1] = m32;
	mat->col[2][2] = m33;
	mat->col[2][3] = m34;

	mat->col[3][0] = m41;
	mat->col[3][1] = m42;
	mat->col[3][2] = m43;
	mat->col[3][3] = m44;
}

static INLINE void mat_cpy(mat_t *dst, const mat_t *src)
{
	memcpy(dst, src, sizeof(*dst));
}

static INLINE void mat_print(const mat_t mat)
{
	printf("[ %+.2f  %+.2f  %+.2f  %+.2f  \n",
		mat.col[0][0], mat.col[1][0], mat.col[2][0], mat.col[3][0]);

	printf("  %+.2f  %+.2f  %+.2f  %+.2f  \n",
		mat.col[0][1], mat.col[1][1], mat.col[2][1], mat.col[3][1]);

	printf("  %+.2f  %+.2f  %+.2f  %+.2f  \n",
		mat.col[0][2], mat.col[1][2], mat.col[2][2], mat.col[3][2]);

	printf("  %+.2f  %+.2f  %+.2f  %+.2f ]\n",
		mat.col[0][3], mat.col[1][3], mat.col[2][3], mat.col[3][3]);
}

END_DECLARATIONS;

#endif
