#include "text.h"
#include "widget.h"
#include "core/memory.h"
#include "core/string.h"
#include "renderer/mesh.h"
#include "renderer/font.h"
#include "resources/resources.h"
#include "math/math.h"
#include <stdio.h>
#include <float.h>
#include <math.h>

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

	text->alignment = ALIGNMENT_CENTRE;
	text->margin.left = 5;
	text->margin.right = 5;
	text->margin.top = 5;
	text->margin.bottom = 5;

	return text;
}

void text_destroy(text_t *text)
{
	if (text == NULL) {
		return;
	}

	if (text->mesh != NULL) {
		mesh_destroy(text->mesh);
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

void text_update_margin(text_t *text, int8_t left, int8_t right, int8_t top, int8_t bottom)
{
	if (text == NULL) {
		return;
	}

	text->margin.left = left;
	text->margin.right = right;
	text->margin.top = top;
	text->margin.bottom = bottom;

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

	for (vindex_t i = 0; i < (vindex_t)text->buffer_size; i++) {

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

int16_t text_calculate_width(text_t *text, int end_index)
{
	if (text == NULL ||
		text->buffer == NULL ||
		text->font == NULL ||
		end_index == 0) {

		return 0.0f;
	}

	// Stop calculation at the last character in the buffer.
	if (end_index < 0 || end_index > (int)text->buffer_length) {
		end_index = (int)text->buffer_length;
	}

	int16_t width = 0.0f;

	for (int i = 0; i < end_index; i++) {

		// Get the glyph for the character at the current position.
		glyph_t *g = font_get_glyph(text->font, (uint8_t)text->buffer[i]);

		// Some glyphs may not have a visual representation, so skip them.
		if (g == NULL) {
			continue;
		}

		// Advance current position.
		width += (int16_t)g->advance.x;
	}

	return width;
}

uint32_t text_get_closest_character(text_t *text, int16_t x, int16_t *out_x)
{
	if (text == NULL || string_is_null_or_empty(text->buffer)) {
		return 0;
	}

	int16_t position = 0;
	int16_t closest_position = 0;
	uint32_t closest = 0;

	for (uint32_t i = 0; i < text->buffer_length; i++) {

		// Get the glyph for the character at the current position.
		glyph_t *g = font_get_glyph(text->font, (uint8_t)text->buffer[i]);

		// Some glyphs may not have a visual representation, so skip them.
		if (g == NULL) {
			continue;
		}

		// Advance current position.
		position += (int16_t)g->advance.x;

		if (position < x) {

			closest_position = position;
			closest = i + 1;
		}
		else {

			// Going further, so we've already found the closest character.
			break;
		}
	}

	// If the position is further to the right than the width of the text, the position is beyond
	// the last character of the text.
	if (x > position) {

		closest = text->buffer_length;
		closest_position = position;
	}

	if (out_x != NULL) {
		*out_x = closest_position;
	}

	return closest;
}

static void text_refresh_vertices(text_t *text)
{
	if (text->font == NULL) {
		return;
	}

	float pos_x = 0;
	float min_y = FLT_MAX;
	float max_y = FLT_MIN;

	vertex_ui_t *vertices = text->mesh->ui_vertices;

	for (size_t i = 0; i < text->buffer_length; i++) {

		// Get the glyph for the character at the current position.
		glyph_t *g = font_get_glyph(text->font, (uint8_t)text->buffer[i]);
		size_t base = i * NUM_VERTICES_PER_CHAR;

		// Some glyphs may not have a visual representation, so clear their vertices.
		// Also clear vertices if they'd go off the boundaries of the text object.
		if (g == NULL ||
		    pos_x + g->advance.x >= text->position.x + text->boundaries.x - text->margin.left) {

			vertices[base + 0] = vertex_ui_empty();
			vertices[base + 1] = vertex_ui_empty();
			vertices[base + 2] = vertex_ui_empty();
			vertices[base + 3] = vertex_ui_empty();
			
			continue;
		}

		// Calculate the position of each glyph. As a comment I am not sure why the Y coordinate
		// works as well as it does, but as long as it does...
		vec2_t p = vec2(pos_x + g->bearing.x, -g->bearing.y);
		vec2_t s = g->size;

		vertices[base + 0] = vertex_ui(vec2(p.x, p.y),               vec2(g->uv1.x, g->uv1.y), text->colour);
		vertices[base + 1] = vertex_ui(vec2(p.x, (p.y + s.y)),       vec2(g->uv1.x, g->uv2.y), text->colour);
		vertices[base + 2] = vertex_ui(vec2(p.x + s.x, p.y),         vec2(g->uv2.x, g->uv1.y), text->colour);
		vertices[base + 3] = vertex_ui(vec2(p.x + s.x, (p.y + s.y)), vec2(g->uv2.x, g->uv2.y), text->colour);

		// Update tallest and shortest coordinates.
		if (p.y + g->bearing.y < min_y) {
			min_y = p.y + g->bearing.y;
		}
		if (p.y + s.y + g->bearing.y > max_y) {
			max_y = p.y + s.y + g->bearing.y;
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

	vec2_t boundaries = vec2i_to_vec2(text->boundaries);

	// Offset text position and boundaries by margins.
	position.x += text->margin.left;
	position.y += text->margin.top;
	boundaries.x -= text->margin.left + text->margin.right;
	boundaries.y -= text->margin.top + text->margin.bottom;

	//
	// --- Horizontal alignment ---
	//
	if (text->alignment & ALIGNMENT_LEFT) {
		// Nothing to do here - the text is aligned from the left bottom corner.
	}
	else if (text->alignment & ALIGNMENT_RIGHT) {
		position.x += (boundaries.x - text->size.x);
	}
	else {
		position.x += (boundaries.x - text->size.x) / 2;
	}

	//
	// --- Vertical alignment ---
	//
	if (text->alignment & ALIGNMENT_TOP) {
		position.y += text->size.y;
	}
	else if (text->alignment & ALIGNMENT_BOTTOM) {
		position.y += boundaries.y;
	}
	else {
		position.y += (boundaries.y + text->size.y) / 2;
	}

	// Offset and flip the vertices.
	float bottom = mgui_parameters.height;

	for (size_t i = 0; i < text->buffer_length; i++) {

		for (size_t j = 0; j < NUM_VERTICES_PER_CHAR; j++) {

			vertex_ui_t *v = &text->mesh->ui_vertices[i * NUM_VERTICES_PER_CHAR + j];

			v->pos = vec2(
				v->pos.x + position.x,
				roundf(bottom - (v->pos.y + position.y)) // Ensure coordinate is in full pixels
			);
		}
	}

	text->cursor_position = vec2(position.x, position.y);
}
