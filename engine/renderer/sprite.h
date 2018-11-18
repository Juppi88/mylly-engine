#pragma once
#ifndef __SPRITE_H
#define __SPRITE_H

#include "core/types.h"
#include "math/vector.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct sprite_t {

	mesh_t *mesh; // Sprite mesh
	texture_t *texture; // Texture this sprite is a part of

	vec2_t position; // The position of the bottom-left corner of the sprite (in pixels)
	vec2_t size; // The size of the sprite (in pixels)
	vec2_t uv1; // Texture coordinates of the bottom-left corner
	vec2_t uv2; // Texture coordinates of the top-right corner
	vec2_t pivot; // Offset of pivot from the centre of the sprite (in pixels)
	float pixels_per_unit; // Number of pixels matching one world unit

} sprite_t;

// -------------------------------------------------------------------------------------------------

sprite_t *sprite_create(texture_t *texture, vec2_t position, vec2_t size,
						vec2_t pivot, float pixels_per_unit);

void sprite_destroy(sprite_t *sprite);

END_DECLARATIONS;

#endif
