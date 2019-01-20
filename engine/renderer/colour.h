#pragma once
#ifndef __COLOUR_H
#define __COLOUR_H

#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

typedef struct colour_t {

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

} colour_t;

// -------------------------------------------------------------------------------------------------

// Constructor macros
#ifndef __cplusplus
	#define col(r, g, b) (colour_t){ r, g, b, 255 }
	#define col_a(r, g, b, a) (colour_t){ r, g, b, a }
#else
	#define col(r, g, b) { r, g, b, 255 }
	#define col_a(r, g, b, a) { r, g, b, a }
#endif

// -------------------------------------------------------------------------------------------------

// Pre-defined colours
#define COL_TRANSPARENT col_a(0, 0, 0, 0)
#define COL_WHITE col(255, 255, 255)
#define COL_BLACK col(0, 0, 0)
#define COL_RED col(255, 0, 0)
#define COL_GREEN col(0, 255, 0)
#define COL_BLUE col(0, 0, 255)
#define COL_YELLOW col(255, 255, 0)

// -------------------------------------------------------------------------------------------------

colour_t col_add(colour_t a, colour_t b);
colour_t col_multiply(colour_t a, colour_t b);

#endif
