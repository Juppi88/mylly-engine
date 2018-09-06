#pragma once
#ifndef __VECTOR_H
#define __VECTOR_H

// --------------------------------------------------------------------------------
// vec2_t

typedef struct vec2_t {
	float x, y;
} vec2_t;

#define vec2(x, y) { x, y }
#define vec2_zero() { 0, 0 }
#define vec2_one() { 1, 1 }

// --------------------------------------------------------------------------------
// vec3_t

typedef struct vec3_t {
	float x, y, z;
} vec3_t;

#define vec3(x, y, z) { x, y, z }
#define vec3_zero() { 0, 0, 0 }
#define vec3_one() { 1, 1, 1 }

float vec3_normalize(vec3_t *v);
vec3_t vec3_normalized(const vec3_t *v);

// --------------------------------------------------------------------------------

#endif
