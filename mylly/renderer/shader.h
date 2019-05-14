#pragma once
#ifndef __SHADER_H
#define __SHADER_H

#include "core/defines.h"
#include "collections/array.h"
#include "resources/resource.h"
#include "math/vector.h"
#include "renderer/colour.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef uint32_t shader_object_t;
typedef uint32_t shader_program_t;

// -------------------------------------------------------------------------------------------------

// - Create arrays for matrix and vec4 uniforms, get their locations and use glUniform4fv
//   and the matrix one to upload the data
// - Have fixed indices to the uniform arrays represent certain types of data.
// - Have a few custom user data entries at the end of the array
// - Define the indices in mylly.gcing or similar
// - Have static arrays in renderer.c and update them before committing arrays to program
// - Colours should be vec4's

// -------------------------------------------------------------------------------------------------

typedef enum {

	SHADER_VERTEX,
	SHADER_FRAGMENT,
	NUM_SHADER_TYPES

} SHADER_TYPE;

// -------------------------------------------------------------------------------------------------

typedef enum {

	QUEUE_BACKGROUND,
	QUEUE_GEOMETRY,
	QUEUE_TRANSPARENT,
	QUEUE_OVERLAY,
	NUM_QUEUES
	
} SHADER_QUEUE;

// -------------------------------------------------------------------------------------------------

typedef enum {

	ATTR_VERTEX,                                                             
	ATTR_NORMAL,
	ATTR_TEXCOORD,
	ATTR_COLOUR,
	ATTR_CENTRE,
	ATTR_SIZE,
	ATTR_TANGENT,
	NUM_SHADER_ATTRIBUTES

} SHADER_ATTRIBUTE;

// -------------------------------------------------------------------------------------------------

typedef enum {

	UNIFORM_TYPE_INT, // A single int
	UNIFORM_TYPE_FLOAT, // A single float
	UNIFORM_TYPE_VECTOR4, // 4-element float vector
	UNIFORM_TYPE_COLOUR, // 4-byte RGBA colour

} UNIFORM_TYPE;

// -------------------------------------------------------------------------------------------------

// Built-in uniform array indices.

enum {
	UNIFORM_MAT_MVP = 0, // Model-view-projection matrix
	UNIFORM_MAT_MODEL = 1, // Model matrix
	UNIFORM_MAT_VIEW = 2, // View matrix
	UNIFORM_MAT_PROJECTION = 3, // Projection matrix

	NUM_MAT_UNIFORMS
};

enum {
	UNIFORM_VEC_VIEW_POSITION = 0, // The position of the rendered view (camera)
	UNIFORM_VEC_TIME = 1, // 4-element vector containing time (see core/time.h)
	UNIFORM_VEC_SCREEN = 2, // Screen size in pixels
	UNIFORM_VEC_COLOUR = 3, // Colour of the mesh. For lit materials this is the ambient colour
	
	NUM_VEC_UNIFORMS
};

enum {
	UNIFORM_SAMPLER_MAIN = 0, // Main sampler (usually diffuse texture)
	UNIFORM_SAMPLER_NORMAL = 1, // Normal map texture
	NUM_SAMPLER_UNIFORMS
};

// -------------------------------------------------------------------------------------------------

// Custom material uniforms.
typedef struct shader_uniform_t {

	char *name; // Uniform name
	UNIFORM_TYPE type; // Data type
	int position; // Shader position

	union { // Value
		int i;
		float f;
		vec4_t vec;
	} value;

} shader_uniform_t;

// -------------------------------------------------------------------------------------------------

typedef struct shader_t {

	resource_t resource; // Resource info

	shader_object_t vertex; // Vertex shader stage
	shader_object_t fragment; // Fragment shader stage
	shader_program_t program; // Combined shader program

	int queue; // Render queue used by this shader - see enum SHADER_QUEUE above
	int attributes[NUM_SHADER_ATTRIBUTES]; // List of vertex attributes used by the program

	// Positions for built-in renderer uniform arrays.
	int matrix_array; // Matrix data
	int vector_array; // Vector data
	int sampler_array; // Textures
	int light_array; // Lights
	int num_lights_position;

	// Positions and values for custom material uniforms.
	arr_t(shader_uniform_t) material_uniforms;
	bool has_updated_uniforms; // A flag indicating whether the custom uniforms need updating

	arr_t(const char *) source; // An array consisting of source code lines of the shader

} shader_t;

// -------------------------------------------------------------------------------------------------

// NOTE: Should not be used directly. Load resources via the res_* API.
shader_t *shader_create(const char *name, const char *path);
shader_t *shader_clone(shader_t *original);
void shader_destroy(shader_t *shader);

void shader_set_uniform_int(shader_t *shader, const char *name, int value);
void shader_set_uniform_float(shader_t *shader, const char *name, float value);
void shader_set_uniform_vector(shader_t *shader, const char *name, vec4_t value);
void shader_set_uniform_colour(shader_t *shader, const char *name, colour_t value);

void shader_set_render_queue(shader_t *shader, SHADER_QUEUE queue);

// -------------------------------------------------------------------------------------------------

bool shader_load_from_source(
	shader_t *shader,
	size_t num_lines, const char **lines,
	size_t num_uniforms, const char **uniforms, UNIFORM_TYPE *uniform_types);

// -------------------------------------------------------------------------------------------------

#define shader_uses_attribute(shader, attribute)\
	((shader)->attributes[(attribute)] >= 0)

#define shader_get_attribute(shader, attribute)\
	((shader)->attributes[(attribute)])

#define shader_is_affected_by_light(shader)\
	((shader)->light_array >= 0 && (shader)->num_lights_position >= 0)

END_DECLARATIONS;

#endif
