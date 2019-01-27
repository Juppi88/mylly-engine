#pragma once
#ifndef __TEXTURE_H
#define __TEXTURE_H

#include "core/defines.h"
#include "resources/resource.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef uint32_t texture_name_t;

// -------------------------------------------------------------------------------------------------

typedef struct texture_t {

	resource_t resource; // Resource info
	uint16_t width; // Texture width in pixels
	uint16_t height; // Texture height in pixels
	texture_name_t gpu_texture; // GPU texture name
	void *data; // Texture pixels

} texture_t;

// -------------------------------------------------------------------------------------------------

// NOTE: Should not be used directly. Load resources via the res_* API.
texture_t *texture_create(const char *name, const char *path);
void texture_destroy(texture_t *texture);

bool texture_load_png(texture_t *texture, void *data, size_t data_length);
bool texture_load_glyph_bitmap(texture_t *texture, uint8_t *data, uint16_t width, uint16_t height);

END_DECLARATIONS;

#endif
