#include "panel.h"
#include "mgui/widget.h"

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

widget_t *panel_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_PANEL;
	widget->callbacks = &callbacks;
	widget->state |= WIDGET_STATE_HAS_MESH;

	return widget;
}
