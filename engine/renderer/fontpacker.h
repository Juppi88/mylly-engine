#pragma once
#ifndef __FONTPACKER_H
#define __FONTPACKER_H

#include "core/defines.h"

typedef struct glyph_t glyph_t;

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

bool create_font_bitmap(glyph_t **glyphs, size_t num_glyphs,
                         uint8_t *bitmap, size_t width, size_t height);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
