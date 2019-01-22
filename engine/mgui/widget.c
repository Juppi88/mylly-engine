#include "widget.h"
#include "core/memory.h"
#include "renderer/mesh.h"
#include "renderer/rendersystem.h"
#include "resources/resources.h"

// -------------------------------------------------------------------------------------------------

static void widget_create_mesh(widget_t *widget);
static void widget_refresh_mesh(widget_t *widget);

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
	list_init(widget->children);

	mgui_add_widget_layer(widget);

	// Allocate the vertices for this widget on the GPU.
	widget_create_mesh(widget);

	return widget;
}

void widget_destroy(widget_t *widget)
{
	if (widget == NULL) {
		return;
	}

	// Remove references to widget.
	if (widget == mgui_get_focused_widget()) {
		mgui_set_focused_widget(NULL);
	}

	// Destroy child widgets.
	widget_t *child, *tmp;

	list_foreach_safe(widget->children, child, tmp) {
		widget_destroy(child);
	}

	// Release used memory.
	mesh_destroy(widget->mesh);

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
		widget_refresh_mesh(widget);
	}

	// Actual rendering is done in the render system - just report that this mesh needs to be
	// rendered during this frame.
	rsys_render_ui_mesh(widget->mesh);

	// Process children.
	widget_t *child;

	list_foreach(widget->children, child) {
		widget_process(child);
	}

	// Reset state flags.
	widget->has_moved = false;
	widget->has_resized = false;
	widget->has_colour_changed = false;
}

static void widget_create_mesh(widget_t *widget)
{
	// Create a new mesh to store the widgets vertices into.
	widget->mesh = mesh_create();

	// Preallocate the vertices. This creates a local copy and a GPU buffer.
	mesh_prealloc_vertices(widget->mesh, VERTEX_UI, NUM_WIDGET_VERTICES);

	// Set initial vertex values.
	widget_refresh_mesh(widget);

	// Set widget vertex indices. This takes into consideration the offset caused by using a shared
	// vertex buffer.
	vindex_t indices[NUM_WIDGET_INDICES] = { 0, 1, 2, 2, 1, 3 };
	mesh_set_indices(widget->mesh, indices, NUM_WIDGET_INDICES);

	// Use a default material.
	mesh_set_material(widget->mesh, res_get_shader("default-ui"), res_get_texture("tiles"));
}

static void widget_refresh_mesh(widget_t *widget)
{
	// Refresh child widgets.
	widget_t *child;

	list_foreach(widget->children, child) {
		widget_refresh_mesh(child);
	}

	vec2_t p = vec2(widget->world_position.x, widget->world_position.y);
	vec2_t s = vec2(widget->size.x, widget->size.y);
	float o = mgui_parameters.height;

	widget->mesh->ui_vertices[0] = vertex_ui(vec2(p.x, o - p.y), vec2(0, 0), widget->colour);
	widget->mesh->ui_vertices[1] = vertex_ui(vec2(p.x + s.x, o - p.y), vec2(1, 0), widget->colour);
	widget->mesh->ui_vertices[2] = vertex_ui(vec2(p.x, o - (p.y + s.y)), vec2(0, 1), widget->colour);
	widget->mesh->ui_vertices[3] = vertex_ui(vec2(p.x + s.x, o - (p.y + s.y)), vec2(1, 1), widget->colour);

	widget->mesh->is_vertex_data_dirty = true;
}

void widget_set_position(widget_t *widget, vec2i_t position)
{
	if (widget == NULL) {
		return;
	}

	widget->position = position;
	widget->has_moved = true;

	// Recalculate world position.
	if (widget->parent != NULL) {
		widget->world_position = vec2i_add(widget->parent->world_position, widget->position);
	}
	else {
		widget->world_position = widget->position;
	}
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

bool widget_is_point_inside(widget_t *widget, vec2i_t point)
{
	if (widget == NULL) {
		return false;
	}

	return (
		point.x >= widget->position.x &&
		point.x <= widget->position.x + widget->size.x &&
		point.y >= widget->position.y &&
		point.y <= widget->position.y + widget->size.y
	);
}

widget_t *widget_get_child_at_position(widget_t *widget, vec2i_t point)
{
	if (widget == NULL) {
		return NULL;
	}

	// Test self first.
	if (!widget_is_point_inside(widget, point)) {
		return NULL;
	}

	// Test each child recursively.
	widget_t *child, *hit;

	list_foreach_reverse(widget->children, child) {
		
		hit = widget_get_child_at_position(child, point);

		if (hit != NULL) {
			return hit;
		}
	}
	
	return widget;
}

widget_t *widget_get_grandparent(widget_t *widget)
{
	if (widget == NULL) {
		return NULL;
	}

	while (widget->parent != NULL) {
		widget = widget->parent;
	}

	return widget;
}
