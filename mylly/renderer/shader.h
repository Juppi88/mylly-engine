#pragma once
#ifndef __SHADER_H
#define __SHADER_H

#include "core/defines.h"
#include "renderer/shaderdata.h"
#include "collections/array.h"
#include "resources/resource.h"

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
	NUM_SHADER_ATTRIBUTES

} SHADER_ATTRIBUTE;

// -------------------------------------------------------------------------------------------------

// Uniform array indices.

enum {
	UNIFORM_MAT_MVP = 0, // Model-view-projection matrix
	UNIFORM_MAT_MODEL = 1, // Model matrix
	UNIFORM_MAT_VIEW = 2, // View matrix
	UNIFORM_MAT_PROJECTION = 3, // Projection matrix
	UNIFORM_MAT_USER1 = 4, // User defined uniform matrices
	UNIFORM_MAT_USER2 = 5,
	UNIFORM_MAT_USER3 = 6,
	UNIFORM_MAT_USER4 = 7,

	NUM_MAT_UNIFORMS
};

enum {
	UNIFORM_VEC_VIEW_POSITION = 0, // The position of the rendered view (camera)
	UNIFORM_VEC_TIME = 1, // 4-element vector containing time (see core/time.h)
	UNIFORM_VEC_SCREEN = 2, // Screen size in pixels
	UNIFORM_VEC_USER1 = 3, // User defined uniform data vectors
	UNIFORM_VEC_USER2 = 4,
	UNIFORM_VEC_USER3 = 5,
	UNIFORM_VEC_USER4 = 6,
	UNIFORM_VEC_USER5 = 7,
	UNIFORM_VEC_USER6 = 8,

	NUM_VEC_UNIFORMS
};

enum {
	UNIFORM_SAMPLER_MAIN = 0, // Main sampler (usually diffuse texture)
	NUM_SAMPLER_UNIFORMS
};

// -------------------------------------------------------------------------------------------------

typedef struct shader_t {

	resource_t resource; // Resource info

	shader_object_t vertex; // Vertex shader stage
	shader_object_t fragment; // Fragment shader stage
	shader_program_t program; // Combined shader program

	int queue; // Render queue used by this shader - see enum SHADER_QUEUE above
	int attributes[NUM_SHADER_ATTRIBUTES]; // List of vertex attributes used by the program

	// Uniform array positions
	int matrix_array;
	int vector_array;
	int sampler_array;

} shader_t;

// -------------------------------------------------------------------------------------------------

// NOTE: Should not be used directly. Load resources via the res_* API.
shader_t * shader_create(const char *name, const char *path);
void shader_destroy(shader_t *shader);

bool shader_load_from_source(shader_t *shader, const char **lines, size_t num_lines);

// -------------------------------------------------------------------------------------------------

#define shader_uses_attribute(shader, attribute)\
	((shader)->attributes[(attribute)] >= 0)

#define shader_get_attribute(shader, attribute)\
	((shader)->attributes[(attribute)])

END_DECLARATIONS;

#endif
