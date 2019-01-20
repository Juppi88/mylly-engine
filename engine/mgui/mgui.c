#include "mgui.h"
#include "widget.h"
#include "collections/list.h"
#include "renderer/buffercache.h"

// -------------------------------------------------------------------------------------------------

mgui_parameters_t mgui_parameters;
static list_t(widget_t) widgets;

// -------------------------------------------------------------------------------------------------

void mgui_initialize(const mgui_parameters_t config)
{
	mgui_parameters = config;
}

void mgui_shutdown(void)
{
	// Destroy widgets which are yet to be destroyed.
	widget_t *widget, *tmp;

	list_foreach_safe(widgets, widget, tmp) {
		widget_destroy(widget);
	}
}

void mgui_process(void)
{
	// TODO: If widgets have been destroyed, the vertex buffer should also be rebuilt eventually
	// to avoid running out of vertices.
	widget_t *widget;

	// Process widgets and report visible ones to the render system for rendering.
	// This is a recursive call and will process all child widgets automatically.
	list_foreach(widgets, widget) {
		widget_process(widget);
	}
}

void mgui_add_widget_layer(widget_t *widget)
{
	// TODO: REMOVE FROM PARENT BEFORE THIS
	list_push(widgets, widget);
}

void mgui_remove_widget_layer(widget_t *widget)
{
	list_remove(widgets, widget);
}
