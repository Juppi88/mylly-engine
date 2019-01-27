#include "font.h"
#include "core/memory.h"
#include "core/string.h"
#include "io/log.h"
#include <ft2build.h>
#include <freetype/ftglyph.h>
#include FT_FREETYPE_H

// -------------------------------------------------------------------------------------------------

font_t *font_create(const char *name, const char *path)
{
	NEW(font_t, font);

	font->resource.name = string_duplicate(name);

	if (path != NULL) {
		font->resource.path = string_duplicate(path);
	}
	
	return font;
}

void font_destroy(font_t *font)
{
	if (font == NULL) {
		return;
	}

	// Destroy glyph data.
	if (font->glyphs != NULL) {
		
		font_destroy_glyph_bitmaps(font);
		DESTROY(font->glyphs);
	}

	DESTROY(font->resource.name);
	DESTROY(font->resource.path);
	DESTROY(font);
}

void font_destroy_glyph_bitmaps(font_t *font)
{
	if (font == NULL || font->glyphs == NULL) {
		return;
	}

	for (uint32_t i = font->first_glyph, c = i + font->num_glyphs; i < c; i++) {

		uint32_t index = i - font->first_glyph;

		if (font->glyphs[index].bitmap != NULL) {

			DESTROY(font->glyphs[index].bitmap->pixels);
			DESTROY(font->glyphs[index].bitmap);
		}
	}
}

bool font_load_from_file(font_t *font, FT_Library freetype,
                         uint8_t size, uint32_t first_glyph, uint32_t last_glyph)
{
	// Validate parameters.
	if (font == NULL || freetype == NULL || size < 4 || last_glyph < first_glyph) {
		return false;
	}

	FT_Face face;

	// Load the font using FreeType.
	if (string_is_null_or_empty(font->resource.path) ||
		FT_New_Face(freetype, font->resource.path, 0, &face)) {

		log_warning("Font", "Failed to load font %s.", font->resource.path);
		return false;
	}

	// Set font size.
	if (FT_Set_Pixel_Sizes(face, 0, size)) {
		
		log_warning("Font", "Failed to set font size of font %s.", font->resource.path);
		return false;
	}

	font->size = size;
	font->first_glyph = first_glyph;
	font->num_glyphs = (last_glyph - first_glyph) + 1;

	// Allocate space for glyph data.
	font->glyphs = mem_alloc(font->num_glyphs * sizeof(glyph_t));

	// Render all glyphs and store their metrics.
	for (uint32_t glyph = first_glyph; glyph <= last_glyph; glyph++) {

		uint32_t index = glyph - first_glyph;
		uint32_t glyph_index = FT_Get_Char_Index(face, glyph);

		// First load the glyph.
		if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {

			log_warning("Font", "Failed to load glyph %u of font %s (%u pt).",
				glyph, font->resource.name, size);

			continue;
		}

		// Render the glyph into a bitmap.
		if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {

			log_warning("Font", "Failed to render glyph %u of font %s (%u pt).",
				glyph, font->resource.name, size);

			continue;
		}

		// Copy the glyph bitmap into temporary memory.
		FT_GlyphSlot g = face->glyph;

		NEW(glyph_bitmap_t, bitmap);

		bitmap->width = g->bitmap.width;
		bitmap->height = g->bitmap.rows;
		bitmap->pixels = mem_alloc_fast(bitmap->width * bitmap->height);

		memcpy(bitmap->pixels, g->bitmap.buffer, bitmap->width * bitmap->height);

		font->glyphs[index].bitmap = bitmap;

		// Store glyph metrics.
		font->glyphs[index].bearing = vec2(g->bitmap_left, g->bitmap_top);
		font->glyphs[index].advance = vec2(g->advance.x / 64.0f, g->advance.y / 64.0f);
	}

	// Release font face.
	FT_Done_Face(face);

	return true;
}
