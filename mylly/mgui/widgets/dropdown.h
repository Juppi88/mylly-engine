#pragma once
#ifndef __MGUI_DROPDOWN_H
#define __MGUI_DROPDOWN_H

#include "mgui/mgui.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

typedef void (*on_dropdown_item_selected_t)(widget_t *dropdown, const char *option, void *data);

// -------------------------------------------------------------------------------------------------

typedef struct dropdown_t {

	sprite_t *arrow; // The icon in the centre when the checkbox is checked
	colour_t arrow_colour; // Colour of the checked icon

	widget_t *background; // The background panel for the opened dropdown list
	widget_t *selection; // The selection background
	widget_t *grid; // The grid widget containing all the options

	int16_t list_height; // Total height of the option list
	int16_t item_padding; // Padding added to each option item

	bool is_opened; // Is the option list open
	
	on_dropdown_item_selected_t on_selected; // Called when the user selects an option
	widget_t *selected_option; // Pointer to the selected option label
	widget_t *hovered_option; // Pointer to the currently hovered option label

} dropdown_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *dropdown_create(widget_t *parent);

void dropdown_set_arrow(widget_t *dropdown, sprite_t *icon);
void dropdown_set_arrow_colour(widget_t *dropdown, colour_t colour);
void dropdown_set_background(widget_t *dropdown, sprite_t *background);
void dropdown_set_background_colour(widget_t *dropdown, colour_t colour);
void dropdown_set_selection(widget_t *dropdown, sprite_t *background);
void dropdown_set_selection_colour(widget_t *dropdown, colour_t colour);

void dropdown_add_option(widget_t *dropdown, const char *option, void *data);

void dropdown_set_selected_handler(widget_t *dropdown, on_dropdown_item_selected_t handler);
void dropdown_get_selected_option(widget_t *dropdown, const char **option, void **data);
void dropdown_select_option(widget_t *dropdown, const char *option);
void dropdown_select_option_by_data(widget_t *dropdown, void *data);

END_DECLARATIONS;

#endif
