#include "grid.h"
#include "mgui/widget.h"

// -------------------------------------------------------------------------------------------------

static void grid_process(widget_t *grid);

// -------------------------------------------------------------------------------------------------
static widget_callbacks_t callbacks = {
	NULL,
	grid_process,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

widget_t *grid_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_GRID;
	widget->callbacks = &callbacks;

	widget->grid.margin = vec2i(0, 0);
	widget->grid.item_size = vec2i(100, 100);
	widget->grid.reposition = true;

	return widget;
}

void grid_reposition(widget_t *grid)
{
	if (grid == NULL || grid->type != WIDGET_TYPE_GRID) {
		return;
	}

	grid->grid.reposition = true;
}

void grid_set_margin(widget_t *grid, vec2i_t margin)
{
	if (grid == NULL || grid->type != WIDGET_TYPE_GRID) {
		return;
	}

	grid->grid.margin = margin;
	grid->grid.reposition = true;
}

void grid_set_item_size(widget_t *grid, vec2i_t size)
{
	if (grid == NULL || grid->type != WIDGET_TYPE_GRID) {
		return;
	}

	grid->grid.item_size = size;
	grid->grid.reposition = true;
}

static void grid_process(widget_t *grid)
{
	if (grid == NULL || grid->type != WIDGET_TYPE_GRID) {
		return;
	}

	// Nothing to do.
	if (!grid->grid.reposition) {
		return;
	}

	// Reorder and reposition child widgets.
	vec2i_t size = grid->grid.item_size;
	vec2i_t margin = grid->grid.margin;
	vec2i_t position = margin;

	widget_t *child;

	list_foreach(grid->children, child) {

		// Place the child item here.
		widget_set_position(child, position);
		widget_set_size(child, size);

		// Move the position for the next item.
		position.x += size.x + margin.x;

		// If the item would exceed the boundaries of the grid, change row.
		if (position.x + size.x + margin.x > grid->size.x) {

			position.x = margin.x;
			position.y += size.y + margin.y;
		}
	}

	grid->grid.reposition = false;
}
