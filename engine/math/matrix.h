#pragma once
#ifndef __MATRIX_H
#define __MATRIX_H

#include "math/vector.h"
#include "core/defines.h"
#include <stdio.h>

// --------------------------------------------------------------------------------
// mat_t - A 4x4 matrix structure

typedef float mat_t[4][4];

#define mat_zero() { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
#define mat_identity() { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } }

#define mat_as_ptr(mat) &mat[0][0]

// --------------------------------------------------------------------------------

void mat_multiply3(const mat_t *mat, const vec3_t *v, vec3_t *out);
void mat_multiply4(const mat_t *mat, const vec4_t *v, vec4_t *out);
void mat_multiply(const mat_t *mat1, const mat_t *mat2, mat_t *out);

// --------------------------------------------------------------------------------

static INLINE void mat_cpy(mat_t *dst, const mat_t *src)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			(*dst)[i][j] = (*src)[i][j];
		}
	}
}

static INLINE void mat_print(const mat_t *mat)
{
	printf("[ %+.2f  %+.2f  %+.2f  %+.2f  \n", (*mat)[0][0], (*mat)[0][1], (*mat)[0][2], (*mat)[0][3]);
	printf("  %+.2f  %+.2f  %+.2f  %+.2f  \n", (*mat)[1][0], (*mat)[1][1], (*mat)[1][2], (*mat)[1][3]);
	printf("  %+.2f  %+.2f  %+.2f  %+.2f  \n", (*mat)[2][0], (*mat)[2][1], (*mat)[2][2], (*mat)[2][3]);
	printf("  %+.2f  %+.2f  %+.2f  %+.2f ]\n", (*mat)[3][0], (*mat)[3][1], (*mat)[3][2], (*mat)[3][3]);
}

#endif