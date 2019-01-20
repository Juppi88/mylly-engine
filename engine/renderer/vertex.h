#pragma once
#ifndef __VERTEX_H
#define __VERTEX_H

#include "core/defines.h"
#include "math/vector.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

typedef uint16_t vindex_t;

// -------------------------------------------------------------------------------------------------

// Specify the type of vertex to be used
typedef enum {

	VERTEX_NORMAL, // Regular 3D vertices with normals
	VERTEX_PARTICLE, // Particle vertices
	VERTEX_UI, // UI panel vertices

} vertex_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct vertex_t {

	vec4_t pos;
	vec3_t normal;
	vec2_t uv;
	colour_t colour;

} vertex_t;

#ifndef __cplusplus
#define vertex_empty() { .colour = COL_WHITE }
#define vertex(pos, normal, uv, colour) (vertex_t){ pos, normal, uv, colour }
#else
#define vertex_empty() { vec4p(0, 0, 0, 1), vec3_zero(), vec2_zero(), COL_WHITE }
#define vertex(pos, normal, uv, colour) { pos, normal, uv, colour }
#endif

// -------------------------------------------------------------------------------------------------

// Vertex definition for particles. These have particle specific attricutes which replace normals.
typedef struct vertex_particle_t {

	vec4_t pos; // Position of the vertex
	vec3_t centre; // Centre of the particle
	vec2_t uv; // Texture coordinates for the particle texture
	colour_t colour; // Colour of the particle
	float size; // Size of the particle quad

} vertex_particle_t;

#define vertex_particle(pos, centre, uv, colour, size) \
	(vertex_particle_t){ pos, centre, uv, colour, size }

// -------------------------------------------------------------------------------------------------

// Vertex definition for UI panels. 2D panels require less information than regular vertices.
typedef struct vertex_ui_t {

	vec2_t pos; // Position of the vertex
	vec2_t uv; // Texture coordinates
	colour_t colour; // Colour of the vertex

} vertex_ui_t;

#define vertex_ui(pos, uv, colour) \
	(vertex_ui_t){ pos, uv, colour }

// -------------------------------------------------------------------------------------------------

#endif
