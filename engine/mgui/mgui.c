#include "mgui.h"
#include "widget.h"
#include "collections/list.h"
#include "renderer/buffercache.h"

// -------------------------------------------------------------------------------------------------

mgui_parameters_t mgui_parameters;

static list_t(widget_t) widgets;
static widget_t *focused_widget;
static widget_t *dragged_widget;

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

widget_t *mgui_get_widget_at_position(vec2i_t point)
{
	widget_t *widget, *hit;

	list_foreach_reverse(widgets, widget) {

		hit = widget_get_child_at_position(widget, point);
		if (hit != NULL) {
			return hit;
		}
	}

	return NULL;
}

widget_t *mgui_get_focused_widget(void)
{
	return focused_widget;
}

void mgui_set_focused_widget(widget_t *widget)
{
	if (widget == focused_widget) {
		return;
	}

	// Remove focus from previous widget.
	// TODO: Callback

	focused_widget = widget;

	if (focused_widget == NULL) {
		return;
	}

	// Change focus to the new widget.
	// TODO: Callback

	// Move the new focused widget (or its grandparent) to the top (i.e. last of the widgets list).
	widget_t *layer = widget_get_grandparent(focused_widget);

	if (layer != NULL &&
		list_contains(widgets, layer)) {

		list_remove(widgets, layer);
		list_push(widgets, layer);
	}
}

widget_t *mgui_get_dragged_widget(void)
{
	return dragged_widget;
}

void mgui_set_dragged_widget(widget_t *widget)
{
	if (widget == dragged_widget) {
		return;
	}

	dragged_widget = widget;

	// Drag the parent widget.
	if (dragged_widget != NULL) {
		dragged_widget = widget_get_grandparent(dragged_widget);
	}
}
