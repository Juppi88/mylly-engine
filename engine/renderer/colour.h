#pragma once
#ifndef __COLOUR_H
#define __COLOUR_H

#include "core/defines.h"

typedef struct colour_t {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} colour_t;

// Constructor macros
#define col(r, g, b) { r, g, b, 255 }
#define col_a(r, g, b, a) { r, g, b, a }

// Pre-defined colours
#define COL_WHITE { 255, 255, 255, 255 }
#define COL_BLACK { 0, 0, 0, 255 }
#define COL_RED { 255, 0, 0, 255 }
#define COL_GREEN { 0, 255, 0, 255 }
#define COL_BLUE { 0, 0, 255, 255 }
#define COL_YELLOW { 255, 255, 0, 255 }

#endif
