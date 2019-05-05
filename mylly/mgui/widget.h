#pragma once
#ifndef __MGUI_WIDGET_H
#define __MGUI_WIDGET_H

#include "mgui/mgui.h"
#include "mgui/widgets/button.h"
#include "mgui/widgets/checkbox.h"
#include "mgui/widgets/grid.h"
#include "mgui/widgets/inputbox.h"
#include "mgui/widgets/label.h"
#include "mgui/widgets/panel.h"
#include "collections/list.h"
#include "renderer/vertex.h"
#include "renderer/buffer.h"

// -------------------------------------------------------------------------------------------------

#define NUM_WIDGET_VERTICES 16
#define NUM_WIDGET_INDICES 54
#define NUM_WIDGET_ADDITIONAL_VERTICES 4
#define NUM_WIDGET_ADDITIONAL_INDICES 6

// -------------------------------------------------------------------------------------------------

typedef struct text_t text_t;

// -------------------------------------------------------------------------------------------------

typedef enum widget_type_t {

	WIDGET_TYPE_WIDGET,
	WIDGET_TYPE_BUTTON,
	WIDGET_TYPE_CHECKBOX,
	WIDGET_TYPE_GRID,
	WIDGET_TYPE_INPUTBOX,
	WIDGET_TYPE_LABEL,
	WIDGET_TYPE_PANEL,

	NUM_WIDGET_TYPES

} widget_type_t;

// -------------------------------------------------------------------------------------------------

typedef enum widget_state_t {

	WIDGET_STATE_INVISIBLE = 0x01,
	WIDGET_STATE_DISABLED = 0x02,
	WIDGET_STATE_FOCUSED = 0x04,
	WIDGET_STATE_HOVERED = 0x08,
	WIDGET_STATE_PRESSED = 0x10,
	WIDGET_STATE_HOVERABLE = 0x20,
	WIDGET_STATE_PRESSABLE = 0x40,
	WIDGET_STATE_CONSUME_CHILD_PRESSES = 0x80, // Consume child clicks (i.e. label of a checkbox)
	WIDGET_STATE_HAS_MESH = 0x100, // Widget has a mesh i.e. it is visible
	WIDGET_STATE_EXT_MESH = 0x200, // Use an extended mesh with space for an additional sprite

} widget_state_t;

// -------------------------------------------------------------------------------------------------

typedef struct widget_callbacks_t {

	void (*on_destroy)(widget_t *widget);
	void (*on_process)(widget_t *widget);
	void (*on_process_text)(widget_t *widget);
	void (*on_refresh_vertices)(widget_t *widget);
	void (*on_focused)(widget_t *widget, bool focused);
	void (*on_hovered)(widget_t *widget, bool hovered);
	void (*on_pressed)(widget_t *widget, bool pressed);
	bool (*on_key_pressed)(widget_t *widget, uint32_t key, bool pressed);
	bool (*on_character_injected)(widget_t *widget, uint32_t c);

} widget_callbacks_t;

// -------------------------------------------------------------------------------------------------

typedef enum anchor_type_t {

	ANCHOR_NONE, // Not anchored to anything
	ANCHOR_MIN, // Left or top
	ANCHOR_MIDDLE, // The centre of the target widget
	ANCHOR_MAX, // Right or bottom

} anchor_type_t;

// -------------------------------------------------------------------------------------------------

typedef enum anchor_edge_t {

	ANCHOR_EDGE_LEFT,
	ANCHOR_EDGE_TOP,
	ANCHOR_EDGE_RIGHT,
	ANCHOR_EDGE_BOTTOM,
	NUM_ANCHOR_EDGES

} anchor_edge_t;

// -------------------------------------------------------------------------------------------------

typedef struct anchor_t {

	anchor_type_t type; // The point where the widget is anchored to
	int16_t offset; // Offset from the anchor point

} anchor_t;

// -------------------------------------------------------------------------------------------------

// Parameters for override widget input handler.
typedef struct widget_event_t {

	widget_t *widget;
	input_event_t type;

	union {
		struct {
			uint32_t key;
		} keyboard;

		struct {
			mouse_button_t button;
			mouse_wheel_t wheel;
			int16_t x;
			int16_t y;
		} mouse;
	};

} widget_event_t;

// -------------------------------------------------------------------------------------------------

// Input event handlers.
typedef void (*on_widget_pressed_t)(widget_t *widget, bool pressed);
typedef void (*on_widget_hovered_t)(widget_t *widget, bool hovered);

// Override event handler for widget input events (mouse movement and key presses).
typedef void (*widget_input_handler_t)(widget_event_t *event);

// -------------------------------------------------------------------------------------------------

typedef struct widget_t {

	list_entry(widget_t);
	struct widget_t *parent;
	list_t(widget_t) children;

	vec2i_t position;
	vec2i_t world_position;
	vec2i_t size;
	colour_t colour;

	vec2i_t bounds_min; // The bounding box of the widget. Takes child widget sizes into account.
	vec2i_t bounds_max;

	anchor_t anchors[NUM_ANCHOR_EDGES]; // Widget anchors

	bool has_moved;
	bool has_resized;
	bool has_colour_changed; // TODO: Name this better!
	bool have_child_boundaries_changed;

	widget_state_t state;

	sprite_t *sprite;
	text_t *text;

	// Widget mesh and material info.
	mesh_t *mesh;

	// Callbacks for certain types of events (internal use only).
	widget_callbacks_t *callbacks;

	// Input handlers.
	on_widget_pressed_t on_pressed;
	on_widget_hovered_t on_hovered;

	void *user_context; // Custom per-widget user context

	// Override input handler.
	widget_input_handler_t input_handler;

	// Widget-type specific data.
	widget_type_t type;

	union {
		button_t button;
		checkbox_t checkbox;
		grid_t grid;
		inputbox_t inputbox;
		label_t label;
		panel_t panel;
	};

} widget_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *widget_create(widget_t *parent);
void widget_destroy(widget_t *widget);
void widget_process(widget_t *widget);

void widget_add_child(widget_t *widget, widget_t *child);
void widget_remove_from_parent(widget_t *widget);

vec2i_t widget_get_position(widget_t *widget);
vec2i_t widget_get_world_position(widget_t *widget);
vec2i_t widget_get_size(widget_t *widget);

void widget_set_position(widget_t *widget, vec2i_t position);
void widget_set_size(widget_t *widget, vec2i_t size);

void widget_set_anchor(widget_t *widget, anchor_edge_t edge, anchor_type_t type, int16_t offset);
void widget_set_anchors(widget_t *widget,
	anchor_type_t left_type, int16_t left_offset,
	anchor_type_t right_type, int16_t right_offset,
	anchor_type_t top_type, int16_t top_offset,
	anchor_type_t bottom_type, int16_t bottom_offset);

void widget_set_colour(widget_t *widget, colour_t colour);
void widget_set_sprite(widget_t *widget, sprite_t *sprite);
void widget_set_visible(widget_t *widget, bool visible);

void widget_set_text(widget_t *widget, const char* format, ...);
void widget_set_text_s(widget_t *widget, const char *text);
void widget_set_text_colour(widget_t *widget, colour_t colour);
void widget_set_text_font(widget_t *widget, font_t *font);
void widget_set_text_alignment(widget_t *widget, alignment_t alignment);
void widget_set_text_margin(widget_t *widget, int8_t left, int8_t right, int8_t top, int8_t bottom);

bool widget_is_point_inside(widget_t *widget, vec2i_t point);
widget_t *widget_get_child_at_position(widget_t *widget, vec2i_t point);

widget_t *widget_get_grandparent(widget_t *widget);

void widget_set_pressed_handler(widget_t *widget, on_widget_pressed_t handler);
void widget_set_hovered_handler(widget_t *widget, on_widget_hovered_t handler);
void widget_set_input_handler(widget_t *widget, widget_input_handler_t handler);
void widget_set_user_context(widget_t *widget, void *context);

END_DECLARATIONS;

#endif
