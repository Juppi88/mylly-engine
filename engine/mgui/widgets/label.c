#include "label.h"
#include "mgui/text.h"
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

widget_t *label_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_LABEL;
	widget->callbacks = &callbacks;
	widget->state |= WIDGET_STATE_NO_MESH;
	widget->text = text_create(widget);

	return widget;
}
