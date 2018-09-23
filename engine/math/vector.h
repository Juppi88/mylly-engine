#pragma once
#ifndef __VECTOR_H
#define __VECTOR_H

#include "core/defines.h"
#include <stdio.h>

// --------------------------------------------------------------------------------
// vec2_t

typedef struct vec2_t {
	float x, y;
} vec2_t;

#define vec2(x, y) { x, y }
#define vec2_zero() { 0, 0 }
#define vec2_one() { 1, 1 }

static INLINE vec2_t vector2(float x, float y)
{
	vec2_t v = vec2(x, y);
	return v;
}

extern vec2_t vec2_zero;
extern vec2_t vec2_one;

// --------------------------------------------------------------------------------
// vec3_t

typedef struct vec3_t {
	float x, y, z;
} vec3_t;

#define vec3(x, y, z) { x, y, z }
#define vec3_zero() { 0, 0, 0 }
#define vec3_one() { 1, 1, 1 }

extern vec3_t vec3_zero;
extern vec3_t vec3_one;

static INLINE vec3_t vector3(float x, float y, float z)
{
	vec3_t v = vec3(x, y, z);
	return v;
}

float vec3_dot(const vec3_t *a, const vec3_t *b);
vec3_t vec3_cross(const vec3_t *a, const vec3_t *b);

float vec3_normalize(vec3_t *v);
vec3_t vec3_normalized(const vec3_t *v);

static INLINE void vec3_print(const vec3_t *vec)
{
	printf("%.2f  %.2f  %.2f\n", vec->x, vec->y, vec->z);
}

// --------------------------------------------------------------------------------
// vec4_t

typedef struct vec4_t {
	float x, y, z, w;
} vec4_t;

#define vec4(x, y, z, w) { x, y, z, w }
#define vec4_zero() { 0, 0, 0, 0 }
#define vec4_one() { 1, 1, 1, 1 }

extern vec4_t vec4_zero;
extern vec4_t vec4_one;

static INLINE vec4_t vector4(float x, float y, float z, float w)
{
	vec4_t v = vec4(x, y, z, w);
	return v;
}

float vec4_normalize(vec4_t *v);
vec4_t vec4_normalized(const vec4_t *v);

// --------------------------------------------------------------------------------

#endif
