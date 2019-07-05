#pragma once
#ifndef __MGUI_H
#define __MGUI_H

#include "core/defines.h"
#include "mgui/vectori.h"

// -------------------------------------------------------------------------------------------------

typedef struct mgui_parameters_t {

	uint16_t width, height;

} mgui_parameters_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void mgui_initialize(const mgui_parameters_t config);
void mgui_shutdown(void);
void mgui_process(void);

// Add or remove a widget layer. Note that calling these will remove the widget from its parent
// and create an independent layer for it.
void mgui_add_widget_layer(widget_t *widget);
void mgui_remove_widget_layer(widget_t *widget);

widget_t *mgui_get_widget_at_position(vec2i_t point);

void mgui_remove_references_to_object(widget_t *widget);
void mgui_add_widget_delayed_render(widget_t *widget);

widget_t *mgui_get_focused_widget(void);
widget_t *mgui_get_dragged_widget(void);
widget_t *mgui_get_hovered_widget(void);
widget_t *mgui_get_pressed_widget(void);

void mgui_set_focused_widget(widget_t *widget);
void mgui_set_dragged_widget(widget_t *widget);
void mgui_set_hovered_widget(widget_t *widget);
void mgui_set_pressed_widget(widget_t *widget, int16_t x, int16_t y);

// -------------------------------------------------------------------------------------------------

// Current set of parameters MGUI has been initialized with.
extern mgui_parameters_t mgui_parameters;

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
