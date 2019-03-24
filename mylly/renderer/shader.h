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

typedef enum {

	SHADER_VERTEX,
	SHADER_FRAGMENT,

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

// Stores a position of a uniform in a shader program along with a reference to uniform data.
typedef struct shader_uniform_position_t {

	int position;
	const shader_uniform_t *uniform;

} shader_uniform_position_t;

// -------------------------------------------------------------------------------------------------

typedef struct shader_t {

	resource_t resource; // Resource info

	shader_object_t vertex; // Vertex shader stage
	shader_object_t fragment; // Fragment shader stage
	shader_program_t program; // Combined shader program

	int queue; // Render queue used by this shader - see enum SHADER_QUEUE above
	int attributes[NUM_SHADER_ATTRIBUTES]; // List of vertex attributes used by the program

	arr_t(shader_uniform_position_t) uniforms; // A list of uniforms used by the shader program.

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
