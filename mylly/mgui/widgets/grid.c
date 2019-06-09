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

void grid_set_max_items_per_row(widget_t *grid, uint32_t num_items)
{
	if (grid == NULL || grid->type != WIDGET_TYPE_GRID) {
		return;
	}

	grid->grid.max_items_per_row = num_items;
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
	uint32_t num_row_items = 0, max_items = grid->grid.max_items_per_row;

	list_foreach(grid->children, child) {

		// Place the child item here.
		widget_set_position(child, position);
		widget_set_size(child, size);
		
		num_row_items++;

		// Move the position for the next item.
		position.x += size.x + margin.x;

		// If the item would exceed the boundaries of the grid, change row. Change row also if there
		// is a maximum number of items per row.
		if (position.x + size.x + margin.x > grid->size.x ||
			(max_items != 0 && num_row_items >= max_items)) {

			position.x = margin.x;
			position.y += size.y + margin.y;
			num_row_items = 0;
		}
	}

	grid->grid.reposition = false;
}
