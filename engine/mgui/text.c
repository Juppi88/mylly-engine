#include "text.h"
#include "widget.h"
#include "core/memory.h"
#include "core/string.h"
#include "renderer/mesh.h"
#include "renderer/font.h"
#include "resources/resources.h"
#include <stdio.h>
#include <float.h>

// -------------------------------------------------------------------------------------------------

static void text_reallocate_vertices(text_t *text);
static void text_refresh_vertices(text_t *text);
static void text_calculate_position(text_t *text);

// -------------------------------------------------------------------------------------------------

text_t *text_create(widget_t *parent)
{
	// A text must always have a widget parent.
	if (parent == NULL) {
		return NULL;
	}

	NEW(text_t, text);

	text->parent = parent;
	text->position = vec2i_zero();
	text->size = vec2i_zero();
	text->boundaries = vec2_to_vec2i(parent->size);
	text->colour = COL_WHITE;
	text->buffer = NULL;
	text->buffer_length = 0;
	text->buffer_size = 0;

	return text;
}

void text_destroy(text_t *text)
{
	if (text == NULL) {
		return;
	}

	DESTROY(text->buffer);
	DESTROY(text);
}

void text_update_buffer(text_t *text, const char *message, size_t length)
{
	if (text == NULL || message == NULL) {
		return;
	}

	// Delete the previous buffer.
	DESTROY(text->buffer);

	// Create a new buffer and copy the message into it.
	text->buffer_length = length;
	text->buffer = mem_alloc(length + 1);

	string_copy(text->buffer, message, length + 1);
	
	text->is_dirty = true;
	text->is_text_dirty = true;
}

void text_update_colour(text_t *text, colour_t colour)
{
	if (text == NULL) {
		return;
	}

	text->colour = colour;
	text->is_dirty = true;
}

void text_update_font(text_t *text, font_t *font)
{
	if (text == NULL || font == NULL) {
		return;
	}

	text->font = font;
	text->is_dirty = true;

	if (text->mesh != NULL &&
		font->texture != NULL) {

		mesh_set_texture(text->mesh, font->texture);
	}
}

void text_update_alignment(text_t *text, alignment_t alignment)
{
	if (text == NULL) {
		return;
	}

	text->alignment = alignment;
	text->is_dirty = true;
}

void text_update_boundaries(text_t *text, vec2i_t position, vec2i_t boundaries)
{
	if (text == NULL) {
		return;
	}

	text->position = position;
	text->boundaries = boundaries;
	text->is_dirty = true;
}

void text_update(text_t *text)
{
	if (text == NULL) {
		return;
	}

	// Ensure a mesh object for the text exists.
	if (text->mesh == NULL) {

		// However if the text hasn't even been set yet, there is no need to update.
		if (!text->is_dirty) {
			return;
		}

		text->mesh = mesh_create();

		// Set mesh material.
		mesh_set_shader(text->mesh, res_get_shader("default-ui-font"));

		if (text->font != NULL) {
			mesh_set_texture(text->mesh, text->font->texture);
		}
	}

	// Check whether the currently allocated buffer is too small for the new text and if so,
	// allocate a bigger buffer.
	if (text->is_text_dirty) {

		if (text->buffer_length > text->buffer_size) {
			text_reallocate_vertices(text);
		}

		// Only draw the indices which currently point to valid and visible glyph vertices.
		text->mesh->num_indices_to_render = NUM_INDICES_PER_CHAR * text->buffer_length;
		text->is_text_dirty = false;
	}

	// Refresh vertices and recalculate text metrics.
	text_refresh_vertices(text);

	text->mesh->is_vertex_data_dirty = true;
	text->is_dirty = false;
}

static void text_reallocate_vertices(text_t *text)
{
	// Allocate a big enough vertex buffer for the text (with some extra for minor changes).
	// Increment buffer size by 20 glyphs at a time.
	const size_t GRANULARITY = 20;

	text->buffer_size =
		text->buffer_length + GRANULARITY - (text->buffer_length % GRANULARITY);

	// Preallocate the vertices. This creates a local copy and a GPU buffer.
	mesh_prealloc_vertices(text->mesh, VERTEX_UI, NUM_VERTICES_PER_CHAR * text->buffer_size);

	// Calculate indices and copy them into the buffer.
	size_t num_indices = NUM_INDICES_PER_CHAR * text->buffer_size;
	NEW_ARRAY(vindex_t, indices, num_indices);

	for (size_t i = 0; i < text->buffer_size; i++) {

		indices[i * NUM_INDICES_PER_CHAR + 0] = i * NUM_VERTICES_PER_CHAR + 0;
		indices[i * NUM_INDICES_PER_CHAR + 1] = i * NUM_VERTICES_PER_CHAR + 1;
		indices[i * NUM_INDICES_PER_CHAR + 2] = i * NUM_VERTICES_PER_CHAR + 2;
		indices[i * NUM_INDICES_PER_CHAR + 3] = i * NUM_VERTICES_PER_CHAR + 2;
		indices[i * NUM_INDICES_PER_CHAR + 4] = i * NUM_VERTICES_PER_CHAR + 1;
		indices[i * NUM_INDICES_PER_CHAR + 5] = i * NUM_VERTICES_PER_CHAR + 3;
	}

	mesh_set_indices(text->mesh, indices, num_indices);

	// Remove temporary index array.
	DESTROY(indices);
}

static void text_refresh_vertices(text_t *text)
{
	float pos_x = 0;
	float min_y = FLT_MAX;
	float max_y = FLT_MIN;

	vertex_ui_t *vertices = text->mesh->ui_vertices;

	for (size_t i = 0; i < text->buffer_length; i++) {

		// Get the glyph for the character at the current position.
		glyph_t *g = font_get_glyph(text->font, (uint8_t)text->buffer[i]);
		size_t base = i * NUM_VERTICES_PER_CHAR;

		// Some glyphs may not have a visual representation, so clear their vertices.
		if (g == NULL) {

			vertices[base + 0] = vertex_ui_empty();
			vertices[base + 1] = vertex_ui_empty();
			vertices[base + 2] = vertex_ui_empty();
			vertices[base + 3] = vertex_ui_empty();
			
			continue;
		}

		// Calculate the position of each glyph. As a comment I am not sure why the Y coordinate
		// works as well as it does, but as long as it does...
		vec2_t p = vec2(pos_x + g->bearing.x, (g->size.y - g->bearing.y) - g->size.y);
		vec2_t s = g->size;

		vertices[base + 0] = vertex_ui(vec2(p.x, p.y),               vec2(g->uv1.x, g->uv1.y), text->colour);
		vertices[base + 1] = vertex_ui(vec2(p.x, (p.y + s.y)),       vec2(g->uv1.x, g->uv2.y), text->colour);
		vertices[base + 2] = vertex_ui(vec2(p.x + s.x, p.y),         vec2(g->uv2.x, g->uv1.y), text->colour);
		vertices[base + 3] = vertex_ui(vec2(p.x + s.x, (p.y + s.y)), vec2(g->uv2.x, g->uv2.y), text->colour);

		// Update tallest and shortest coordinates.
		if (p.y < min_y) {
			min_y = p.y;
		}
		if (p.y + s.y > max_y) {
			max_y = p.y + s.y;
		}
		
		// Advance current position.
		pos_x += g->advance.x;
	}

	// Re-calculate text size.
	text->size = vec2i(
		(int16_t)pos_x,
		(int16_t)(max_y - min_y)
	);

	// Move the text to a correct position in relation to its parent and alignment.
	text_calculate_position(text);
}

static void text_calculate_position(text_t *text)
{
	vec2_t position = vec2_add(
		vec2i_to_vec2(text->parent->world_position),
		vec2i_to_vec2(text->position)
	);

	// Horizontal alignment
	if (text->alignment & ALIGNMENT_LEFT) {
		// Nothing to do here - the text is aligned from the left bottom corner.
	}
	else if (text->alignment & ALIGNMENT_RIGHT) {
		position.x += (text->boundaries.x - text->size.x);
	}
	else {
		position.x += (text->boundaries.x - text->size.x) / 2;
	}

	// Vertical alignment
	if (text->alignment & ALIGNMENT_TOP) {
		position.y += text->size.y;
	}
	else if (text->alignment & ALIGNMENT_BOTTOM) {
		position.y += text->boundaries.y;
	}
	else {
		position.y += (text->boundaries.y + text->size.y) / 2;
	}

	// Offset the vertices.
	float bottom = mgui_parameters.height;

	for (size_t i = 0; i < text->buffer_length; i++) {

		for (size_t j = 0; j < NUM_VERTICES_PER_CHAR; j++) {

			vertex_ui_t *v = &text->mesh->ui_vertices[i * NUM_VERTICES_PER_CHAR + j];
			v->pos = vec2(v->pos.x + position.x, bottom - (v->pos.y + position.y));
		}
	}
}
