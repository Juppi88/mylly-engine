#include "label.h"
#include "mgui/text.h"
#include "mgui/widget.h"

// -------------------------------------------------------------------------------------------------

static void label_on_process_text(widget_t *label);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	label_on_process_text,
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
	widget->text = text_create(widget);

	return widget;
}

void label_set_resize_automatically(widget_t *label, bool resize)
{
	if (label == NULL) {
		return;
	}

	label->label.resize_automatically = resize;

	// Update the size of the widget to the size of the text.
	if (resize) {
		label->size = label->text->size;
		label->has_resized = true;
	}
}

static void label_on_process_text(widget_t *label)
{
	if (label == NULL) {
		return;
	}

	if (label->label.resize_automatically) {

		label->size.x = label->text->size.x;
		label->has_resized = true;
	}
}
