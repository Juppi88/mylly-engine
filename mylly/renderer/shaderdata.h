#pragma once
#ifndef __SHADERDATA_H
#define __SHADERDATA_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

// Built-in shader uniform indices. Independent of material definitions.
typedef enum shader_uniform_index_t {

	UNIFORM_MODEL_MATRIX, // The model matrix
	UNIFORM_MVP_MATRIX, // Model-view-projection matrix
	UNIFORM_TEXTURE, // Texture unit
	UNIFORM_TIME, // 4-element vector containing time (see core/time.h)
	UNIFORM_SCREEN, // Screen size in pixels

	UNIFORM_MATERIAL_DEFINED // First index of custom uniforms defined by materials

} shader_uniform_index_t;

// -------------------------------------------------------------------------------------------------

// Shader uniform data types supported by the renderer.
typedef enum shader_data_type_t {

	DATATYPE_INT, // A single int
	DATATYPE_FLOAT, // A single float
	DATATYPE_VECTOR4, // 4-element float vector
	DATATYPE_MATRIX, // 4x4 matrix
	DATATYPE_COLOUR, // 4-byte RGBA colour

} shader_data_type_t;

// -------------------------------------------------------------------------------------------------

// Shader uniform data.
typedef struct shader_uniform_t {

	uint32_t index; // Either one of shader_uniform_index_t or UNIFORM_MATERIAL_DEFINED + {0, 1, ...}
	const char *name; // Name of the uniform in the shader
	shader_data_type_t type; // Data type

} shader_uniform_t;

// -------------------------------------------------------------------------------------------------

void shader_data_initialize(void);
void shader_data_shutdown(void);

uint32_t shader_data_get_uniform_count(void);

// Add a new shader uniform. Returns the index of the created uniform.
uint32_t shader_data_add_uniform(const char *name, shader_data_type_t type);

// Return shader uniform data for a uniform defined by index.
const shader_uniform_t *shader_data_get_uniform_by_index(uint32_t index);

END_DECLARATIONS;

#endif
