#pragma once
#ifndef __LIGHT_H
#define __LIGHT_H

#include "core/types.h"
#include "math/vector.h"
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

	// Spot light properties
	float angle;
	vec3_t direction;

} light_t;

// -------------------------------------------------------------------------------------------------

light_t *light_create(object_t *parent);
void light_destroy(light_t *light);

END_DECLARATIONS;

#endif
