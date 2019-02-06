#include "group.h"
#include "mgui/widget.h"

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

widget_t *group_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_GROUP;
	widget->callbacks = &callbacks;
	widget->state |= WIDGET_STATE_NO_MESH;

	return widget;
}
