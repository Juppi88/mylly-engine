#pragma once
#ifndef __LIGHT_H
#define __LIGHT_H

#include "core/types.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "renderer/colour.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef enum light_type_t {

	LIGHT_DIRECTIONAL,
	LIGHT_POINT,
	LIGHT_SPOT,

} light_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct light_t {

	object_t *parent; // The object this light is attached to
	uint32_t scene_index; // Index of this light in the scene

	light_type_t type;
	colour_t colour;
	float range;
	float intensity;

	// Spotlight properties
	float cutoff_angle;
	float cutoff_angle_outer;
	vec3_t direction;

	// Light information as a matrix to be passed to a shader.
	mat_t shader_params;
	bool is_dirty;

} light_t;

// -------------------------------------------------------------------------------------------------

light_t *light_create(object_t *parent);
void light_destroy(light_t *light);

void light_set_type(light_t *light, light_type_t type);
void light_set_colour(light_t *light, colour_t colour);
void light_set_intensity(light_t *light, float intensity);
void light_set_range(light_t *light, float range);

// Directional and spotlight methods.
void light_set_direction(light_t *light, vec3_t direction);
void light_set_spotlight_cutoff_angle(light_t *light, float angle, float outer_angle);

// Render system methods.
mat_t light_get_shader_params(light_t *light);

END_DECLARATIONS;

#endif
