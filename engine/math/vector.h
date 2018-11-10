#pragma once
#ifndef __VECTOR_H
#define __VECTOR_H

#include "core/defines.h"
#include <stdio.h>

BEGIN_DECLARATIONS;

// --------------------------------------------------------------------------------
// vec2_t

typedef union vec2_t {
	struct { float x, y; };
	float vec[2];
} vec2_t;

#define vec2(x, y) { .vec = { x, y } }
#define vec2_zero() { .vec = { 0, 0 } }
#define vec2_one() { .vec = { 1, 1 } }

static INLINE vec2_t vector2(float x, float y)
{
	vec2_t v = vec2(x, y);
	return v;
}

vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_subtract(vec2_t a, vec2_t b);
vec2_t vec2_multiply(vec2_t v, float value);

extern vec2_t vec2_zero;
extern vec2_t vec2_one;

// --------------------------------------------------------------------------------
// vec3_t

typedef union vec3_t {
	struct { float x, y, z; };
	float vec[3];
} vec3_t;

#define vec3(x, y, z) { .vec = { x, y, z } }
#define vec3_zero() { .vec = { 0, 0, 0 } }
#define vec3_one() { .vec = { 1, 1, 1 } }

extern vec3_t vec3_zero;
extern vec3_t vec3_one;
extern vec3_t vec3_right;
extern vec3_t vec3_up;
extern vec3_t vec3_forward;

static INLINE vec3_t vector3(float x, float y, float z)
{
	vec3_t v = vec3(x, y, z);
	return v;
}

vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_subtract(vec3_t a, vec3_t b);
vec3_t vec3_multiply(vec3_t v, float value);

float vec3_dot(vec3_t a, vec3_t b);
vec3_t vec3_cross(vec3_t a, vec3_t b);

float vec3_normalize(vec3_t *v);
vec3_t vec3_normalized(vec3_t v);

void vec3_orthonormalize(vec3_t *a, vec3_t *b);

vec3_t vec3_sanitize_rotation(vec3_t v);

static INLINE void vec3_print(vec3_t vec)
{
	printf("%.2f  %.2f  %.2f\n", vec.x, vec.y, vec.z);
}

// --------------------------------------------------------------------------------
// vec4_t

typedef union vec4_t {
	struct { float x, y, z, w; };
	float vec[4];
} vec4_t;

#define vec4(x, y, z, w) { .vec = { x, y, z, w } }
#define vec4_zero() { .vec = { 0, 0, 0, 0 } }
#define vec4_one() { .vec = { 1, 1, 1, 1 } }

extern vec4_t vec4_zero;
extern vec4_t vec4_one;

static INLINE vec4_t vector4(float x, float y, float z, float w)
{
	vec4_t v = vec4(x, y, z, w);
	return v;
}

float vec4_normalize(vec4_t v);
vec4_t vec4_normalized(vec4_t v);

static INLINE void vec4_print(vec4_t vec)
{
	printf("%.2f  %.2f  %.2f  %.2f\n", vec.x, vec.y, vec.z, vec.w);
}

// --------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
