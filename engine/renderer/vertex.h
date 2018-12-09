#pragma once
#ifndef __VERTEX_H
#define __VERTEX_H

#include "core/defines.h"
#include "math/vector.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

typedef uint16_t vindex_t;

// -------------------------------------------------------------------------------------------------

typedef struct vertex_t {
	vec4_t pos;
	vec3_t normal;
	vec2_t uv;
	colour_t colour;
} vertex_t;

#define vertex_empty() \
	{ .colour = COL_WHITE }

#define vertex(pos, normal, uv, col) \
	(vertex_t){ pos, normal, uv, col }

// -------------------------------------------------------------------------------------------------

#endif
