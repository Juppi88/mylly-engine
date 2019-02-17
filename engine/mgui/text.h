#pragma once
#ifndef __MGUI_TEXT_H
#define __MGUI_TEXT_H

#include "mgui/mgui.h"
#include "collections/list.h"
#include "renderer/vertex.h"
#include "renderer/buffer.h"

// -------------------------------------------------------------------------------------------------

#define NUM_VERTICES_PER_CHAR 4
#define NUM_INDICES_PER_CHAR 6

// -------------------------------------------------------------------------------------------------

typedef struct text_t {

	widget_t *parent; // Widget the text is a part of

	vec2i_t position; // Bottom left corner of the text
	vec2i_t size; // Size of the text
	vec2i_t boundaries; // Size of text boundaries used for alignment
	alignment_t alignment; // Text alignment

	colour_t colour; // Text colour
	font_t *font; // Text font
	char *buffer; // Text buffer
	size_t buffer_length; // Length of the text in the buffer
	size_t buffer_size; // The maximum text length due to allocated vertices

	bool is_text_dirty; // Contents of the text buffer have changed
	bool is_dirty; // Text properties (colour, font) has changed
	bool has_moved; // The text or its parent has moved or resized

	mesh_t *mesh; // Text mesh and material.

} text_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

text_t *text_create(widget_t *parent);
void text_destroy(text_t *text);

void text_update_buffer(text_t *text, const char *message, size_t length);
void text_update_colour(text_t *text, colour_t colour);
void text_update_font(text_t *text, font_t *font);
void text_update_alignment(text_t *text, alignment_t alignment);
void text_update_boundaries(text_t *text, vec2i_t position, vec2i_t boundaries);

void text_update(text_t *text);

END_DECLARATIONS;

#endif
