#pragma once
#ifndef __FONT_H
#define __FONT_H

#include "core/defines.h"
#include "resources/resource.h"
#include "math/vector.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct FT_LibraryRec_ *FT_Library;

// -------------------------------------------------------------------------------------------------

// A temporary container for font glyph bitmaps.
typedef struct glyph_bitmap_t {

	uint8_t *pixels; // 8-bit greyscale image of the glyph
	uint32_t width; // Width of bitmap (in pixels)
	uint32_t height; // Height of bitmap (in pixels)

} glyph_bitmap_t;

// -------------------------------------------------------------------------------------------------

typedef struct glyph_t {

	vec2_t uv1; // Texture coordinates for the glyph
	vec2_t uv2;
	vec2_t bearing; // Glyph bearing from the cursor (in pixels)
	vec2_t advance; // The amount of pixels the cursor should advance for a next character
	glyph_bitmap_t *bitmap; // Temporary bitmap container (will only be valid during font loading)

} glyph_t;

// -------------------------------------------------------------------------------------------------

typedef struct font_t {

	resource_t resource; // Resource info
	
	uint8_t size; // Font size in points
	uint32_t first_glyph; // The first glyph in the glyph array
	uint32_t num_glyphs; // Number of glyphs in the glyph array
	glyph_t *glyphs; // Font glyphs

} font_t;

// -------------------------------------------------------------------------------------------------

// NOTE: Should not be used directly. Load resources via the res_* API.
font_t *font_create(const char *name, const char *path);
void font_destroy(font_t *font);
void font_destroy_glyph_bitmaps(font_t *font);

bool font_load_from_file(font_t *font, FT_Library freetype,
                         uint8_t size, uint32_t first_glyph, uint32_t last_glyph);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
