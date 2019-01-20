#include "mgui.h"
#include "widget.h"
#include "collections/list.h"
#include "renderer/buffercache.h"

// -------------------------------------------------------------------------------------------------

static mgui_parameters_t parameters;
static list_t(widget_t) widgets;

// -------------------------------------------------------------------------------------------------

void mgui_initialize(const mgui_parameters_t config)
{
	parameters = config;
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
	// Clear the UI index buffer for rebuilding.
	// TODO: If widgets have been destroyed, the vertex buffer should also be rebuilt eventually
	// to avoid running out of vertices.
	bufcache_clear_all_indices(BUFIDX_UI);

	widget_t *widget;

	// Process widgets and add visible ones to the index buffer for rendering.
	// This is a recursive call and will process all child widgets automatically.
	list_foreach(widgets, widget) {

		widget_process(widget);
		widget_render(widget);
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
