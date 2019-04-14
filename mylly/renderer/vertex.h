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
	VERTEX_DEBUG, // Debug object vertex

} vertex_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct vertex_t {

	vec3_t pos;
	vec3_t normal;
	vec3_t tangent;
	vec2_t uv;

} vertex_t;

#ifndef __cplusplus
#define vertex(pos, normal, uv) (vertex_t){ pos, normal, vec3_forward(), uv }
#else
#define vertex(pos, normal, uv) { pos, normal, vec3_forward(), uv }
#endif

// -------------------------------------------------------------------------------------------------

// Vertex definition for particles. These have particle specific attricutes which replace normals.
typedef struct vertex_particle_t {

	vec3_t pos; // Position of the vertex
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

#define vertex_ui_empty()\
	(vertex_ui_t){ vec2_zero(), vec2_zero(), COL_TRANSPARENT }

// -------------------------------------------------------------------------------------------------

// Vertex definition for 3D scene debug primitives.
typedef struct vertex_debug_t {

	vec3_t pos; // Position of the vertex in the scene
	colour_t colour; // Colour of the vertex
	
} vertex_debug_t;

#define vertex_debug(pos, colour) \
	(vertex_debug_t){ pos, colour }

// -------------------------------------------------------------------------------------------------

#endif
