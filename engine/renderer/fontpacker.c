#include "fontpacker.h"
#include "font.h"
#include "io/log.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

static int compare_glyphs(const void *p1, const void *p2);

// -------------------------------------------------------------------------------------------------

bool create_font_texture(glyph_t **glyphs, size_t num_glyphs,
                         uint8_t *bitmap, size_t width, size_t height)
{
	if (glyphs == NULL || num_glyphs == 0 || bitmap == NULL) {
		return false;
	}

	// Sort the glyps by height, tallest first.
	qsort(glyphs, num_glyphs, sizeof(glyph_t*), compare_glyphs);

	// Fit the glyphs into the texture bitmap by using a next-fit decreasing height algorithm.
	// See http://cgi.csc.liv.ac.uk/~epa/surveyhtml.html for more information.
	const size_t PADDING = 2;

	size_t x = PADDING;
	size_t y = PADDING;
	size_t level_height = glyphs[0]->bitmap->height;

	for (size_t i = 0; i < num_glyphs; i++) {

		glyph_bitmap_t *glyph = glyphs[i]->bitmap;

		// Don't try to fit too wide bitmaps into the texture.
		if (glyph->width > width) {

			log_warning("Fontpacker", "Attempting to fit a glyph too wide.");
			continue;
		}

		// Can the glyph be fit into the current level?
		if (x + glyph->width > width) {

			// If not, the current level is closed and a new level must be created.
			x = 0;
			y += level_height + PADDING;
			
			// The height of the next level is always the height of the first glyph
			// due to having the glyphs sorted from tallest to shortest.
			level_height = glyph->height;

			// Ensure the texture has enough space for another level.
			if (y + level_height > height) {

				log_warning("Fontpacker", "Maximum glyps on the texture reached.");
				break;
			}
		}

		// Copy the bitmap of the glyph into the texture.
		for (size_t j = 0; j < glyph->height; j++) {

			for (size_t i = 0; i < glyph->width; i++) {

				bitmap[(x + i) + (y + j) * width] = glyph->pixels[i + j * glyph->width];
			}
		}

		// Calculate UV coordinates for the glyph.
		glyphs[i]->uv1 = vec2((float)(x + glyph->width) / width, (float)(y + glyph->height) / height);
		glyphs[i]->uv2 = vec2((float)x / width, (float)y / height);

		// Advance forward.
		x += glyph->width + PADDING;
	}

	return true;
}

static int compare_glyphs(const void *p1, const void *p2)
{
	const glyph_t *glyph1 = *(glyph_t **)p1;
	const glyph_t *glyph2 = *(glyph_t **)p2;

	if (glyph1->bitmap->height > glyph2->bitmap->height) return -1;
	if (glyph1->bitmap->height < glyph2->bitmap->height) return 1;
	if (glyph1->bitmap->width > glyph2->bitmap->width) return -1;
	if (glyph1->bitmap->width < glyph2->bitmap->width) return 1;

	return 0;
}
