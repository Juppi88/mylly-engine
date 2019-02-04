#pragma once
#ifndef __MGUI_CHECKBOX_H
#define __MGUI_CHECKBOX_H

#include "mgui/mgui.h"
#include "mgui/vector.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

typedef void (*on_checkbox_toggled_t)(void *context);

// -------------------------------------------------------------------------------------------------

typedef struct checkbox_t {

	bool is_toggled; // True when the checkbox is toggled
	sprite_t *icon; // The icon in the centre when the checkbox is checked
	colour_t icon_colour; // Colour of the checked icon

	// Click handler
	on_checkbox_toggled_t on_toggled;
	void *on_toggled_context;

} checkbox_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *checkbox_create(widget_t *parent);

void checkbox_set_toggled(widget_t *checkbox, bool toggled);
void checkbox_set_icon(widget_t *checkbox, sprite_t *icon);
void checkbox_set_icon_colour(widget_t *checkbox, colour_t colour);
void checkbox_set_toggled_handler(widget_t *checkbox, on_checkbox_toggled_t handler, void *context);

END_DECLARATIONS;

#endif
