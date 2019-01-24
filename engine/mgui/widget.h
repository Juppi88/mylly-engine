#pragma once
#ifndef __WIDGET_H
#define __WIDGET_H

#include "mgui/mgui.h"
#include "mgui/vector.h"
#include "collections/list.h"
#include "renderer/vertex.h"
#include "renderer/buffer.h"

// -------------------------------------------------------------------------------------------------

#define NUM_WIDGET_VERTICES 16
#define NUM_WIDGET_INDICES 54

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
	list_t(widget_t) children;

	vec2i_t position;
	vec2i_t world_position;
	vec2i_t size;
	colour_t colour;

	bool has_moved;
	bool has_resized;
	bool has_colour_changed;

	sprite_t *sprite;

	// Widget mesh and material info.
	mesh_t *mesh;

} widget_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *widget_create(void);
void widget_destroy(widget_t *widget);
void widget_process(widget_t *widget);

void widget_add_child(widget_t *widget, widget_t *child);
void widget_remove_from_parent(widget_t *widget);

void widget_set_position(widget_t *widget, vec2i_t position);
void widget_set_size(widget_t *widget, vec2i_t size);
void widget_set_colour(widget_t *widget, colour_t colour);
void widget_set_sprite(widget_t *widget, sprite_t *sprite);

bool widget_is_point_inside(widget_t *widget, vec2i_t point);
widget_t *widget_get_child_at_position(widget_t *widget, vec2i_t point);

widget_t *widget_get_grandparent(widget_t *widget);

END_DECLARATIONS;

#endif
