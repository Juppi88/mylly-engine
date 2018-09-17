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

static INLINE void mat_cpy(mat_t *mat, const mat_t *other)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			(*mat)[i][j] = (*other)[i][j];
		}
	}
}

static INLINE void mat_print(const mat_t *mat)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("mat[%d,%d] = %.2f\n", i, j, (*mat)[i][j]);
		}
	}
}

#endif
