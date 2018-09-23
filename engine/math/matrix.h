#pragma once
#ifndef __MATRIX_H
#define __MATRIX_H

#include "math/vector.h"
#include "core/defines.h"
#include <stdio.h>
#include <string.h>

// --------------------------------------------------------------------------------
// mat_t - A 4x4 matrix structure

typedef struct mat_t {
	float col[4][4]; // Column vectors
} mat_t;

#define mat_zero() { { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } }
#define mat_identity() { { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } } }

extern mat_t mat_identity;

#define mat_as_ptr(mat) &(mat).col[0][0]

// --------------------------------------------------------------------------------

void mat_multiply3(const mat_t *mat, const vec3_t *v, vec3_t *out);
void mat_multiply4(const mat_t *mat, const vec4_t *v, vec4_t *out);
void mat_multiply(const mat_t *mat1, const mat_t *mat2, mat_t *out);

// --------------------------------------------------------------------------------

static INLINE void mat_cpy(mat_t *dst, const mat_t *src)
{
	memcpy(dst, src, sizeof(*dst));
}

static INLINE void mat_print(const mat_t *mat)
{
	printf("[ %+.2f  %+.2f  %+.2f  %+.2f  \n",
		mat->col[0][0], mat->col[1][0], mat->col[2][0], mat->col[3][0]);

	printf("  %+.2f  %+.2f  %+.2f  %+.2f  \n",
		mat->col[0][1], mat->col[1][1], mat->col[2][1], mat->col[3][1]);

	printf("  %+.2f  %+.2f  %+.2f  %+.2f  \n",
		mat->col[0][2], mat->col[1][2], mat->col[2][2], mat->col[3][2]);

	printf("  %+.2f  %+.2f  %+.2f  %+.2f ]\n",
		mat->col[0][3], mat->col[1][3], mat->col[2][3], mat->col[3][3]);
}

#endif
