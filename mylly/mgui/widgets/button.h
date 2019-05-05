#pragma once
#ifndef __MGUI_BUTTON_H
#define __MGUI_BUTTON_H

#include "mgui/mgui.h"
#include "renderer/colour.h"
#include "platform/timer.h"

// -------------------------------------------------------------------------------------------------

typedef void (*on_button_clicked_t)(widget_t *widget);

// -------------------------------------------------------------------------------------------------

typedef struct button_t {

	colour_t normal_colour;
	colour_t hovered_colour;
	colour_t pressed_colour;

	// Colour interpolation when hovering the button.
	timeout_t colour_timer;
	colour_t colour_from;
	colour_t colour_to;

	// Click handler
	on_button_clicked_t on_clicked;
	void *on_clicked_context;

} button_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *button_create(widget_t *parent);

void button_set_colours(widget_t *button, colour_t normal, colour_t hovered, colour_t pressed);
void button_set_clicked_handler(widget_t *button, on_button_clicked_t handler);

END_DECLARATIONS;

#endif
