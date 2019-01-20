#include "widget.h"
#include "core/memory.h"
#include "renderer/buffercache.h"

// -------------------------------------------------------------------------------------------------

static void widget_allocate_vertices(widget_t *widget);
static void widget_refresh_vertices(widget_t *widget);

// -------------------------------------------------------------------------------------------------

widget_t *widget_create(void)
{
	NEW(widget_t, widget);

	widget->position = vec2i_zero();
	widget->world_position = vec2i_zero();
	widget->size = vec2i(100, 100); // Use a non-zero default size
	widget->type = TYPE_WIDGET;
	widget->colour = COL_WHITE;

	widget->has_moved = false;
	widget->has_resized = false;

	// Create a new layer for the widget.
	widget->parent = NULL;
	arr_init(widget->children);

	mgui_add_widget_layer(widget);

	// Allocate the vertices for this widget on the GPU.
	widget->handle_vertices = 0;
	widget->handle_indices = 0;

	widget_allocate_vertices(widget);

	return widget;
}

void widget_destroy(widget_t *widget)
{
	if (widget == NULL) {
		return;
	}

	// Destroy child widgets.
	widget_t *child;

	arr_foreach_reverse(widget->children, child) {
		widget_destroy(child);
	}

	// Release used memory.
	arr_clear(widget->children);
	DESTROY(widget);
}

void widget_process(widget_t *widget)
{
	if (widget == NULL) {
		return;
	}

	// Check whether the parent has changed in a way that would affect this widget.
	if (widget->parent != NULL &&
		widget->parent->has_moved) {

		// Recalculate world position.
		widget->world_position = vec2i_add(widget->parent->world_position, widget->position);
	}

	// Keep vertices up to date if changes in the shape or position of the widget occur.
	if (widget->has_moved || widget->has_resized || widget->has_colour_changed) {
		widget_refresh_vertices(widget);
	}

	// Process children.
	widget_t *child;

	arr_foreach(widget->children, child) {
		widget_process(child);
	}

	// Reset state flags.
	widget->has_moved = false;
	widget->has_resized = false;
	widget->has_colour_changed = false;
}

void widget_render(widget_t *widget)
{
	if (widget == NULL) {
		return;
	}

	// Actual rendering is done in the render system.
	// Here we just move the widget indices to the GPU.
	widget->handle_indices = bufcache_alloc_indices(BUFIDX_UI, &widget->indices,
		                                            NUM_WIDGET_INDICES);

	// Render children as well.
	widget_t *child;

	arr_foreach(widget->children, child) {
		widget_process(child);
	}
}

static void widget_allocate_vertices(widget_t *widget)
{
	// Refresh the local copy of vertices.
	widget_refresh_vertices(widget);

	// Allocate a GPU buffer for the vertices.
	widget->handle_vertices = bufcache_alloc_vertices(BUFIDX_UI, &widget->vertices,
		                                              sizeof(vertex_ui_t), NUM_WIDGET_VERTICES);

	// Calculate indices.
	// GPU allocation of indices is done when rendering the widget.
	uint16_t index_offset = BUFFER_GET_START_INDEX(widget->handle_vertices, sizeof(vertex_ui_t));

	widget->indices[0] = index_offset + 0;
	widget->indices[1] = index_offset + 1;
	widget->indices[2] = index_offset + 2;
	widget->indices[3] = index_offset + 2;
	widget->indices[4] = index_offset + 1;
	widget->indices[5] = index_offset + 3;
}

static void widget_refresh_vertices(widget_t *widget)
{
	// Refresh child widgets.
	widget_t *child;

	arr_foreach(widget->children, child) {
		widget_refresh_vertices(child);
	}

	vec2i_t p = widget->world_position;
	vec2i_t s = widget->size;

	widget->vertices[0] = vertex_ui(vec2(p.x, p.y), vec2(0, 0), widget->colour);
	widget->vertices[1] = vertex_ui(vec2(p.x + s.x, p.y), vec2(1, 0), widget->colour);
	widget->vertices[2] = vertex_ui(vec2(p.x, p.y - s.y), vec2(0, 1), widget->colour);
	widget->vertices[3] = vertex_ui(vec2(p.x + s.x, p.y - s.y), vec2(1, 1), widget->colour);

	// Update the GPU object if it has been allocated.
	bufcache_update(widget->handle_vertices, &widget->vertices);
}

void widget_set_position(widget_t *widget, vec2i_t position)
{
	if (widget == NULL) {
		return;
	}

	widget->position = position;
	widget->has_moved = true;

	// Recalculate world position.
	widget->world_position = vec2i_add(widget->parent->world_position, widget->position);
}

void widget_set_size(widget_t *widget, vec2i_t size)
{
	if (widget == NULL) {
		return;
	}

	widget->size = size;
	widget->has_resized = true;
}

void widget_set_colour(widget_t *widget, colour_t colour)
{
	if (widget == NULL) {
		return;
	}

	widget->colour = colour;
	widget->has_colour_changed = true;
}
