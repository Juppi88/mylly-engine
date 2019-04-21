#pragma once
#ifndef __MATERIAL_H
#define __MATERIAL_H

#include "core/defines.h"
#include "collections/array.h"
#include "resources/resource.h"
#include "renderer/colour.h"
#include "renderer/shader.h"
#include "math/vector.h"

// -------------------------------------------------------------------------------------------------

// Material parameter. Contains a reference to a shader uniform definition with the value for it.
typedef struct material_param_t {

	const char *name; // Name of the parameter and matching uniform in the shader
	UNIFORM_TYPE type; // Type of the parameter

	union { // Parameter value
		int i;
		float f;
		vec4_t vec;
	} value;

} material_param_t;

// -------------------------------------------------------------------------------------------------

typedef struct material_t {

	resource_t resource; // Resource info

	arr_t(material_param_t) parameters; // List of shader parameters defined by this material

	shader_t *shader; // Material shader
	texture_t *texture; // Texture/diffuse map. This is the main texture of the material.
	texture_t *normal_map; // Normal map texture

} material_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

material_t *material_create(const char *name, const char *path);
void material_destroy(material_t *material);

void material_apply_parameters(material_t *material);

END_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

#endif
