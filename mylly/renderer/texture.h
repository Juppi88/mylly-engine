#pragma once
#ifndef __TEXTURE_H
#define __TEXTURE_H

#include "core/defines.h"
#include "collections/array.h"
#include "resources/resource.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef uint32_t texture_name_t;

// -------------------------------------------------------------------------------------------------

// The format of the texture to be uploaded to the GPU.
typedef enum {

	TEX_FORMAT_RGB,
	TEX_FORMAT_RGBA,
	TEX_FORMAT_GRAYSCALE,
	
} TEX_FORMAT;

// -------------------------------------------------------------------------------------------------

// How to interpolate a scaled texture.
typedef enum {

	TEX_FILTER_POINT,
	TEX_FILTER_BILINEAR,
	
} TEX_FILTER;

// -------------------------------------------------------------------------------------------------

typedef struct texture_t {

	resource_t resource; // Resource info
	uint16_t width; // Texture width in pixels
	uint16_t height; // Texture height in pixels
	texture_name_t gpu_texture; // GPU texture name
	void *data; // Texture pixels

	arr_t(sprite_t*) sprites; // List of sprites onthis texture

} texture_t;

// -------------------------------------------------------------------------------------------------

// NOTE: Should not be used directly. Load resources via the res_* API.
texture_t *texture_create(const char *name, const char *path);
void texture_destroy(texture_t *texture);

bool texture_load_png(texture_t *texture, void *data, size_t data_length, TEX_FILTER filter);
bool texture_load_glyph_bitmap(texture_t *texture, uint8_t *data, uint16_t width, uint16_t height);

END_DECLARATIONS;

#endif
