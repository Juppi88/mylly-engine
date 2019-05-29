#pragma once
#ifndef __MGUI_COLOURPICKER_H
#define __MGUI_COLOURPICKER_H

#include "mgui/mgui.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

typedef void (*on_colour_selected_t)(widget_t *picker, colour_t colour);

// -------------------------------------------------------------------------------------------------

typedef struct colourpicker_t {

	colour_t colour; // Currently selected colour
	on_colour_selected_t on_selected; // Called when a colour is clicked
	float brightness; // The brightness of the selector colour map
	float alpha; // The alpha of the selector colour map

} colourpicker_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *colourpicker_create(widget_t *parent);

colour_t colourpicker_get_colour(widget_t *picker);
void colourpicker_set_colour(widget_t *picker, colour_t colour);
void colourpicker_set_brightness_alpha(widget_t *picker, float brightness, float alpha);

void colourpicker_set_selected_handler(widget_t *picker, on_colour_selected_t handler);

// -------------------------------------------------------------------------------------------------

// NOTE: These are internal engine methods, not to be called directly.
void colourpicker_create_texture(void);
void colourpicker_destroy_texture(void);

END_DECLARATIONS;

#endif
