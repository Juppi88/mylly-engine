#pragma once
#ifndef __MGUI_TEXT_H
#define __MGUI_TEXT_H

#include "mgui/mgui.h"
#include "mgui/vector.h"
#include "collections/list.h"
#include "renderer/vertex.h"
#include "renderer/buffer.h"

// -------------------------------------------------------------------------------------------------

#define NUM_VERTICES_PER_CHAR 4
#define NUM_INDICES_PER_CHAR 6

// -------------------------------------------------------------------------------------------------

typedef struct text_t {

	widget_t *parent;

	vec2i_t position;
	vec2i_t size;
	colour_t colour;
	font_t *font;
	char *buffer; // Text buffer
	size_t buffer_length; // Length of the text in the buffer
	size_t buffer_size; // The maximum text length due to allocated vertices

	bool is_dirty;
	bool is_text_dirty;

	// Text mesh and material.
	mesh_t *mesh;

} text_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

text_t *text_create(widget_t *parent);
void text_destroy(text_t *text);

void text_update_buffer(text_t *text, const char *message, size_t length);
void text_update_colour(text_t *text, colour_t colour);
void text_update_font(text_t *text, font_t *font);

void text_update(text_t *text);

END_DECLARATIONS;

#endif
