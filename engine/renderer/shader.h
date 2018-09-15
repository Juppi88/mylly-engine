#pragma once
#ifndef __SHADER_H
#define __SHADER_H

#include "core/defines.h"

// --------------------------------------------------------------------------------

typedef uint32_t shader_object_t;
typedef uint32_t shader_program_t;

typedef enum {
	CONST_MODEL_MATRIX,
	CONST_MODEL_MVP,
	NUM_SHADER_GLOBALS
} SHADER_GLOBAL;

typedef enum {
	SHADER_VERTEX,
	SHADER_FRAGMENT,
} SHADER_TYPE;

// --------------------------------------------------------------------------------

typedef struct shader_t {

	char *name; // Name of the shader

	shader_object_t vertex; // Vertex shader stage
	shader_object_t fragment; // Fragment shader stage
	shader_program_t program; // Combined shader program

	int globals[NUM_SHADER_GLOBALS]; // List of shader constants used by the program
} shader_t;

// --------------------------------------------------------------------------------

shader_t * shader_create(const char *name, const char *source);
void shader_destroy(shader_t *shader);


#define shader_uses_global(shader, global)\
	((shader)->globals[(global)] >= 0)

#define shader_get_global_position(shader, global)\
	((shader)->globals[(global)])

#endif
