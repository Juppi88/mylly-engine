#pragma once
#ifndef __WIDGET_H
#define __WIDGET_H

#include "mgui/mgui.h"
#include "mgui/vector.h"
#include "collections/array.h"
#include "collections/list.h"
#include "renderer/vertex.h"
#include "renderer/buffer.h"

// -------------------------------------------------------------------------------------------------

#define NUM_WIDGET_VERTICES 4
#define NUM_WIDGET_INDICES 6

// -------------------------------------------------------------------------------------------------

typedef enum widget_type_t {

	TYPE_WIDGET,
	NUM_TYPES

} widget_type_t;

// -------------------------------------------------------------------------------------------------

typedef struct widget_t {

	widget_type_t type;

	list_entry(widget_t);
	struct widget_t *parent;
	arr_t(struct widget_t*) children;

	vec2i_t position;
	vec2i_t world_position;
	vec2i_t size;
	colour_t colour;

	bool has_moved;
	bool has_resized;
	bool has_colour_changed;

	// Local copy of UI widget vertices.
	vertex_ui_t vertices[NUM_WIDGET_VERTICES];
	vindex_t indices[NUM_WIDGET_INDICES];

	// Handles to GPU copy of vertices.
	buffer_handle_t handle_vertices;
	buffer_handle_t handle_indices;

} widget_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *widget_create(void);
void widget_destroy(widget_t *widget);
void widget_process(widget_t *widget);

void widget_render(widget_t *widget);

void widget_set_position(widget_t *widget, vec2i_t position);
void widget_set_size(widget_t *widget, vec2i_t size);
void widget_set_colour(widget_t *widget, colour_t colour);

END_DECLARATIONS;

#endif
