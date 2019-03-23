#pragma once
#ifndef __MGUITYPES_H
#define __MGUITYPES_H

// -------------------------------------------------------------------------------------------------

typedef struct widget_t widget_t;
typedef struct text_t text_t;

// -------------------------------------------------------------------------------------------------

typedef enum alignment_t {

	ALIGNMENT_CENTRE = 0, // Horizontally/vertically centered
	ALIGNMENT_LEFT = 0x1, // Aligned to left edge
	ALIGNMENT_RIGHT = 0x2, // Aligned to right edge
	ALIGNMENT_TOP = 0x4, // Vertically aligned to top
	ALIGNMENT_BOTTOM = 0x8, // Vertically aligned to bottom
	ALIGNMENT_TOPLEFT = (ALIGNMENT_TOP | ALIGNMENT_LEFT),
	ALIGNMENT_TOPRIGHT = (ALIGNMENT_TOP | ALIGNMENT_RIGHT),
	ALIGNMENT_BOTTOMLEFT = (ALIGNMENT_BOTTOM | ALIGNMENT_LEFT),
	ALIGNMENT_BOTTOMRIGHT = (ALIGNMENT_BOTTOM | ALIGNMENT_RIGHT),
	
} alignment_t;

// -------------------------------------------------------------------------------------------------

#endif
