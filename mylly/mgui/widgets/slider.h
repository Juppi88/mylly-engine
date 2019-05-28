#pragma once
#ifndef __MGUI_SLIDER_H
#define __MGUI_SLIDER_H

#include "mgui/mgui.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

typedef void (*on_slider_value_changed_t)(widget_t *slider);

// -------------------------------------------------------------------------------------------------

typedef struct slider_t {

	float value; // Current value of the slider
	float min_value; // Value range
	float max_value;
	int num_steps; // Number of steps the range is divided into

	sprite_t *knob; // The sprite of the knob
	colour_t knob_colour; // Colour of the knob

	on_slider_value_changed_t on_changed; // Called when the value is changed

} slider_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *slider_create(widget_t *parent);

float slider_get_value(widget_t *slider);
void slider_set_value(widget_t *slider, float value);
void slider_set_range(widget_t *slider, float min_value, float max_value);
void slider_set_steps(widget_t *slider, int num_steps);

void slider_set_knob_sprite(widget_t *slider, sprite_t *sprite);
void slider_set_knob_colour(widget_t *slider, colour_t colour);
void slider_set_value_changed_handler(widget_t *slider, on_slider_value_changed_t handler);

END_DECLARATIONS;

#endif
