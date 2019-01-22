#pragma once
#ifndef __MGUI_H
#define __MGUI_H

#include "core/defines.h"
#include "mgui/vector.h"

// -------------------------------------------------------------------------------------------------

typedef struct widget_t widget_t;

// -------------------------------------------------------------------------------------------------

typedef struct mgui_parameters_t {

	uint16_t width, height;

} mgui_parameters_t;

// -------------------------------------------------------------------------------------------------

extern mgui_parameters_t mgui_parameters;

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

widget_t *mgui_get_focused_widget(void);
void mgui_set_focused_widget(widget_t *widget);

END_DECLARATIONS;

#endif
