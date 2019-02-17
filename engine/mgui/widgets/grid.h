#pragma once
#ifndef __MGUI_GRID_H
#define __MGUI_GRID_H

#include "mgui/mgui.h"

// -------------------------------------------------------------------------------------------------

typedef struct grid_t {

	vec2i_t margin; // Margin between items in the grid
	vec2i_t item_size; // Size of each child item in the grid
	bool reposition; // Set to true when the child widgets need to be repositioned

} grid_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *grid_create(widget_t *parent);

void grid_reposition(widget_t *grid);

void grid_set_margin(widget_t *grid, vec2i_t margin);
void grid_set_item_size(widget_t *grid, vec2i_t size);

END_DECLARATIONS;

#endif
