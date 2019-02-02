#include "widget.h"
#include "text.h"
#include "core/memory.h"
#include "renderer/mesh.h"
#include "renderer/rendersystem.h"
#include "resources/resources.h"
#include "scene/sprite.h"
#include <string.h>
#include <stdarg.h>

// -------------------------------------------------------------------------------------------------

static void widget_create_mesh(widget_t *widget);
static void widget_refresh_mesh(widget_t *widget);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

widget_t *widget_create(widget_t *parent)
{
	NEW(widget_t, widget);

	widget->position = vec2i_zero();
	widget->world_position = vec2i_zero();
	widget->size = vec2i(100, 100); // Use a non-zero default size
	widget->type = WIDGET_TYPE_WIDGET;
	widget->colour = COL_WHITE;

	widget->has_moved = false;
	widget->has_resized = false;

	widget->callbacks = &callbacks;

	list_init(widget->children);

	if (parent != NULL) {

		// Attach the widget to its new parent.
		list_push(parent->children, widget);
		widget->parent = parent;

		// Update the widget's position.
		widget->world_position = parent->world_position;
	}
	else {

		// Create a new layer for the widget.
		widget->parent = NULL;
		mgui_add_widget_layer(widget);
	}
	
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
	mgui_remove_references_to_object(widget);

	// Destroy child widgets.
	widget_t *child, *tmp;

	list_foreach_safe(widget->children, child, tmp) {
		widget_destroy(child);
	}

	// Release used memory.
	mesh_destroy(widget->mesh);

	if (widget->text != NULL) {
		text_destroy(widget->text);
	}

	DESTROY(widget);
}

void widget_process(widget_t *widget)
{
	if (widget == NULL || (widget->state & WIDGET_STATE_INVISIBLE)) {
		return;
	}

	if (widget->callbacks->on_process != NULL) {
		widget->callbacks->on_process(widget);
	}

	// Check whether the parent has changed in a way that would affect this widget.
	if (widget->parent != NULL &&
		widget->parent->has_moved) {

		// Recalculate world position.
		widget->world_position = vec2i_add(widget->parent->world_position, widget->position);
		widget->has_moved = true;
	}

	// Keep vertices up to date if changes in the shape or position of the widget occur.
	if (widget->has_moved || widget->has_resized || widget->has_colour_changed) {
		widget_refresh_mesh(widget);
	}

	// Update text object.
	if (widget->text != NULL &&
		(widget->has_moved || widget->has_resized || widget->text->is_dirty)) {
		
		text_update(widget->text);
	}

	// Actual rendering is done in the render system - just report that this mesh needs to be
	// rendered during this frame.
	if (widget->sprite != NULL) {
		rsys_render_ui_mesh(widget->mesh);
	}

	// Render text object.
	if (widget->text != NULL &&
		widget->text->mesh != NULL &&
		widget->text->mesh->num_indices != 0) {

		rsys_render_ui_mesh(widget->text->mesh);
	}

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

void widget_add_child(widget_t *widget, widget_t *child)
{
	if (widget == NULL || child == NULL) {
		return;
	}

	// Remove the child object from its previous parent or layer.
	if (child->parent != NULL) {
		list_remove(child->parent->children, child);
	}
	else {
		mgui_remove_widget_layer(child);
	}

	// Attach the child to its new parent.
	list_push(widget->children, child);
	child->parent = widget;

	// Update widget position.
	child->position = vec2i_subtract(child->world_position, widget->world_position);
}

void widget_remove_from_parent(widget_t *widget)
{
	if (widget == NULL || widget->parent == NULL) {
		return;
	}

	// Remove the widget from the parent.
	list_remove(widget->parent->children, widget);

	// Create a new layer for the widget.
	widget->parent = NULL;
	mgui_add_widget_layer(widget);

	// Update widget position.
	widget->position = widget->world_position;
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

	if (widget->text != NULL) {
		text_update_boundaries(widget->text, widget->text->position, vec2_to_vec2i(size));
	}
}

void widget_set_colour(widget_t *widget, colour_t colour)
{
	if (widget == NULL) {
		return;
	}

	widget->colour = colour;
	widget->has_colour_changed = true;
}

void widget_set_sprite(widget_t *widget, sprite_t *sprite)
{
	if (widget == NULL) {
		return;
	}

	widget->sprite = sprite;
	widget->has_colour_changed = true;

	// Change the texture used for rendering.
	mesh_set_texture(widget->mesh, sprite->texture);
}

void widget_set_text(widget_t *widget, const char* format, ...)
{
	if (widget == NULL || widget->text == NULL || format == NULL) {
		return;
	}

	va_list	marker;
	char buffer[1024];

	va_start(marker, format);
	int length = vsnprintf(buffer, sizeof(buffer), format, marker);
	va_end(marker);

	// Ensure buffer formatting succeeded.
	if (length < 0 || (size_t)length >= sizeof(buffer)) {
		return;
	}

	text_update_buffer(widget->text, buffer, (size_t)length);
}

void widget_set_text_s(widget_t *widget, const char *text)
{
	if (widget == NULL || widget->text == NULL || text == NULL) {
		return;
	}

	size_t length = strlen(text);
	text_update_buffer(widget->text, text, length);
}

void widget_set_text_colour(widget_t *widget, colour_t colour)
{
	if (widget == NULL || widget->text == NULL) {
		return;
	}

	text_update_colour(widget->text, colour);
}

void widget_set_text_font(widget_t *widget, font_t *font)
{
	if (widget == NULL || widget->text == NULL || font == NULL) {
		return;
	}

	text_update_font(widget->text, font);
}

void widget_set_text_alignment(widget_t *widget, alignment_t alignment)
{
	if (widget == NULL || widget->text == NULL) {
		return;
	}

	text_update_alignment(widget->text, alignment);
}

bool widget_is_point_inside(widget_t *widget, vec2i_t point)
{
	if (widget == NULL) {
		return false;
	}

	return (
		point.x >= widget->world_position.x &&
		point.x <= widget->world_position.x + widget->size.x &&
		point.y >= widget->world_position.y &&
		point.y <= widget->world_position.y + widget->size.y
	);
}

widget_t *widget_get_child_at_position(widget_t *widget, vec2i_t point)
{
	if (widget == NULL || (widget->state & WIDGET_STATE_INVISIBLE)) {
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

static void widget_create_mesh(widget_t *widget)
{
	// Create a new mesh to store the widgets vertices into.
	widget->mesh = mesh_create();

	// Preallocate the vertices. This creates a local copy and a GPU buffer.
	mesh_prealloc_vertices(widget->mesh, VERTEX_UI, NUM_WIDGET_VERTICES);

	// Set initial vertex values.
	widget_refresh_mesh(widget);

	// Create the indices for a nine-sliced sprite.
	vindex_t indices[NUM_WIDGET_INDICES] = {
		4, 5, 6, 6, 5, 7, // Centre
		0, 8, 15, 15, 8, 4, // Bottom left
		8, 9, 4, 4, 9, 5, // Left
		9, 1, 5, 5, 1, 10, // Top left
		5, 10, 7, 7, 10, 11, // Top
		7, 11, 12, 12, 11, 3, // Top right
		6, 7, 13, 13, 7, 12, // Right
		14, 6, 2, 2, 6, 13, // Bottom right
		15, 4, 14, 14, 4, 6 // Bottom
	};

	// Set widget vertex indices. This takes into consideration the offset caused by using a shared
	// vertex buffer.
	mesh_set_indices(widget->mesh, indices, NUM_WIDGET_INDICES);

	// Use a default shader for rendering the widget.
	mesh_set_shader(widget->mesh, res_get_shader("default-ui"));
}

static void widget_refresh_mesh(widget_t *widget)
{
	if (widget->sprite == NULL) {
		return;
	}

	sprite_t *sprite = widget->sprite;

	vec2_t uv1 = sprite->uv1;
	vec2_t uv2 = sprite->uv2;
	vec2_t suv1 = sprite->slice_uv1;
	vec2_t suv2 = sprite->slice_uv2;
	vec2_t p = vec2i_to_vec2(widget->world_position);

	float bottom = mgui_parameters.height;
	float w = widget->size.x;
	float w1 = sprite->slice_position.x - sprite->position.x; // Width offset from left
	float w2 = sprite->size.x - w1 - sprite->slice_size.x; // Width offset from right
	float h = widget->size.y;
	float h1 = sprite->slice_position.y - sprite->position.y; // Height offset from bottom
	float h2 = sprite->size.y - h1 - sprite->slice_size.y; // Height offset from top

	vertex_ui_t *vertices = widget->mesh->ui_vertices;

	vertices[0]  = vertex_ui(vec2(p.x, bottom - p.y),                     vec2(uv1.x, uv1.y), widget->colour);
	vertices[1]  = vertex_ui(vec2(p.x, bottom - (p.y + h)),               vec2(uv1.x, uv2.y), widget->colour);
	vertices[2]  = vertex_ui(vec2(p.x + w, bottom - p.y),                 vec2(uv2.x, uv1.y), widget->colour);
	vertices[3]  = vertex_ui(vec2(p.x + w, bottom - (p.y + h)),           vec2(uv2.x, uv2.y), widget->colour);

	vertices[4]  = vertex_ui(vec2(p.x + w1, bottom - (p.y + h1)),         vec2(suv1.x, suv1.y), widget->colour);
	vertices[5]  = vertex_ui(vec2(p.x + w1, bottom - (p.y + h - h2)),     vec2(suv1.x, suv2.y), widget->colour);
	vertices[6]  = vertex_ui(vec2(p.x + w - w2, bottom - (p.y + h1)),     vec2(suv2.x, suv1.y), widget->colour);
	vertices[7]  = vertex_ui(vec2(p.x + w - w2, bottom - (p.y + h - h2)), vec2(suv2.x, suv2.y), widget->colour);

	vertices[8]  = vertex_ui(vec2(p.x, bottom - (p.y + h1)),              vec2(uv1.x, suv1.y), widget->colour);
	vertices[9]  = vertex_ui(vec2(p.x, bottom - (p.y + h - h2)),          vec2(uv1.x, suv2.y), widget->colour);
	vertices[10] = vertex_ui(vec2(p.x + w1, bottom - (p.y + h)),          vec2(suv1.x, uv2.y), widget->colour);
	vertices[11] = vertex_ui(vec2(p.x + w - w2, bottom - (p.y + h)),      vec2(suv2.x, uv2.y), widget->colour);

	vertices[12] = vertex_ui(vec2(p.x + w, bottom - (p.y + h - h2)),      vec2(uv2.x, suv2.y), widget->colour);
	vertices[13] = vertex_ui(vec2(p.x + w, bottom - (p.y + h1)),          vec2(uv2.x, suv1.y), widget->colour);
	vertices[14] = vertex_ui(vec2(p.x + w - w2, bottom - (p.y)),          vec2(suv2.x, uv1.y), widget->colour);
	vertices[15] = vertex_ui(vec2(p.x + w1, bottom - (p.y)),              vec2(suv1.x, uv1.y), widget->colour);

	widget->mesh->is_vertex_data_dirty = true;
}
