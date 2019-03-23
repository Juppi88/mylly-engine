#pragma once
#ifndef __MGUI_VECTORI_H
#define __MGUI_VECTORI_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct vec2i_t {
	int16_t x, y;
} vec2i_t;

// -------------------------------------------------------------------------------------------------

#if !defined(__cplusplus) || !defined(_MSC_VER)
	#define vec2i(x, y) (vec2i_t){ x, y }
	#define vec2i_zero() (vec2i_t){ 0, 0 }
#else
	#define vec2i(x, y) { x, y }
	#define vec2i_zero() { 0, 0 }
#endif

#define vec2i_to_vec2(v) vec2((v).x, (v).y)
#define vec2_to_vec2i(v) vec2i((int16_t)(v).x, (int16_t)(v).y)

// -------------------------------------------------------------------------------------------------

vec2i_t vec2i_add(vec2i_t a, vec2i_t b);
vec2i_t vec2i_subtract(vec2i_t a, vec2i_t b);
vec2i_t vec2i_multiply(vec2i_t v, float value);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
