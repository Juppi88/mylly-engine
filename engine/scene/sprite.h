#pragma once
#ifndef __SPRITE_H
#define __SPRITE_H

#include "core/types.h"
#include "math/vector.h"
#include "resources/resource.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct sprite_t {

	resource_t resource; // Resource info

	mesh_t *mesh; // Sprite mesh
	texture_t *texture; // Texture this sprite is a part of

	vec2_t position; // The position of the bottom-left corner of the sprite (in pixels)
	vec2_t size; // The size of the sprite (in pixels)
	vec2_t uv1; // Texture coordinates of the bottom-left corner
	vec2_t uv2; // Texture coordinates of the top-right corner
	vec2_t pivot; // Offset of pivot from the centre of the sprite (in pixels)
	float pixels_per_unit; // Number of pixels matching one world unit

	bool is_nine_slice; // Set to true if the sprite is 9-sliced
	vec2_t slice_position; // The position of the bottom-left corner of the inner rectangle
	vec2_t slice_size; // The size of the inner rectangle
	vec2_t slice_uv1; // Texture coordinates of the bottom-left corner of the inner rectangle
	vec2_t slice_uv2; // Texture coordinates of the top-right corner of the inner rectangle

} sprite_t;

// -------------------------------------------------------------------------------------------------

sprite_t *sprite_create(texture_t *parent, const char *name);
void sprite_destroy(sprite_t *sprite);

void sprite_set(sprite_t *sprite, texture_t *texture,
                vec2_t position, vec2_t size, vec2_t pivot, float pixels_per_unit);

void sprite_set_nine_slice(sprite_t *sprite, vec2_t slice_position, vec2_t slice_size);

END_DECLARATIONS;

#endif
