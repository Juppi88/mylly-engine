#pragma once
#ifndef __MATERIAL_H
#define __MATERIAL_H

#include "core/defines.h"
#include "renderer/shaderdata.h"
#include "collections/array.h"
#include "resources/resource.h"
#include "renderer/colour.h"
#include "math/vector.h"

// -------------------------------------------------------------------------------------------------

// Material parameter value. The datatypes here must correspond with shader_data_type_t,
// with the exception of matrix which isn't supported by custom materials.
typedef union material_value_t {

	int int_value;
	float float_value;
	colour_t colour_value;
	vec4_t vec4_value;

} material_value_t;

// -------------------------------------------------------------------------------------------------

// Material parameter. Contains a reference to a shader uniform definition with the value for it.
typedef struct material_param_t {

	const shader_uniform_t *uniform;
	material_value_t value;

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

END_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

#endif
