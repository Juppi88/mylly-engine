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

#ifndef __cplusplus
#define vec2(x, y) (vec2_t){ .vec = { x, y } }
#define vec2_zero() (vec2_t){ .vec = { 0, 0 } }
#define vec2_one() (vec2_t){ .vec = { 1, 1 } }
#else
#define vec2(x, y) { x, y }
#define vec2_zero() { 0, 0 }
#define vec2_one() { 1, 1 }
#endif

static INLINE vec2_t vector2(float x, float y)
{
	vec2_t v = vec2(x, y);
	return v;
}

vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_subtract(vec2_t a, vec2_t b);
vec2_t vec2_multiply(vec2_t v, float value);

float vec2_dot(vec2_t a, vec2_t b);
float vec2_cross(vec2_t a, vec2_t b);

float vec2_normalize(vec2_t *v);
vec2_t vec2_normalized(vec2_t v);

bool vec2_is_zero(vec2_t v);

static INLINE void vec2_print(vec2_t vec)
{
	printf("%.2f  %.2f\n", vec.x, vec.y);
}

// --------------------------------------------------------------------------------
// vec3_t

typedef union vec3_t {
	struct { float x, y, z; };
	float vec[3];
} vec3_t;

#ifndef __cplusplus
#define vec3(x, y, z) (vec3_t){ .vec = { x, y, z } }
#define vec3_zero() (vec3_t){ .vec = { 0, 0, 0 } }
#define vec3_one() (vec3_t){ .vec = { 1, 1, 1 } }
#define vec3_right() (vec3_t){ .vec = { 1, 0, 0 } }
#define vec3_up() (vec3_t){ .vec = { 0, 1, 0 } }
#define vec3_forward() (vec3_t){ .vec = { 0, 0, 1 } }
#else
#define vec3(x, y, z) { x, y, z }
#define vec3_zero() { 0, 0, 0 }
#define vec3_one() { 1, 1, 1 }
#define vec3_right() { 1, 0, 0 }
#define vec3_up() { 0, 1, 0 }
#define vec3_forward() { 0, 0, 1 }
#endif

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

#ifndef __cplusplus
#define vec4(x, y, z, w) (vec4_t){ .vec = { x, y, z, w } }
#define vec4p(x, y, z) (vec4_t){ .vec = { x, y, z, 1 } }
#define vec4_zero() (vec4_t){ .vec = { 0, 0, 0, 0 } }
#define vec4_one() (vec4_t){ .vec = { 1, 1, 1, 1 } }
#else
#define vec4(x, y, z, w) { x, y, z, w }
#define vec4p(x, y, z) { x, y, z, 1 }
#define vec4_zero() { 0, 0, 0, 0 }
#define vec4_one() { 1, 1, 1, 1 }
#endif

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
