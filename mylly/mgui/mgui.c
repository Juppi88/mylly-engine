#include "mgui.h"
#include "widget.h"
#include "collections/list.h"
#include "collections/array.h"
#include "renderer/buffercache.h"

// -------------------------------------------------------------------------------------------------

mgui_parameters_t mgui_parameters;

static list_t(widget_t) widgets;
static widget_t *focused_widget;
static widget_t *dragged_widget;
static widget_t *hovered_widget;
static widget_t *pressed_widget;

static arr_t(widget_t *) delayed_render_widgets = arr_initializer;

static uint16_t screen_width, screen_height;
static uint16_t min_width, min_height;

// -------------------------------------------------------------------------------------------------

void mgui_initialize(const mgui_parameters_t config)
{
	mgui_parameters = config;

	screen_width = config.width;
	screen_height = config.height;

	// Per-component type initialization.
	colourpicker_create_texture();
}

void mgui_shutdown(void)
{
	// Per-component type cleanup.
	colourpicker_destroy_texture();

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
		widget_render(widget);
	}

	// Some widgets may want a delayed render, and they'll indicate their wish each frame.
	// Draw delayed widgets.
	if (delayed_render_widgets.count != 0) {

		arr_foreach(delayed_render_widgets, widget) {
			widget_render(widget);	
		}

		arr_clear(delayed_render_widgets);
	}
}

void mgui_set_min_resolution(uint16_t width, uint16_t height)
{
	min_width = width;
	min_height = height;

	if (mgui_parameters.width < min_width) {
		mgui_parameters.width = min_width;
	}

	if (mgui_parameters.height < min_height) {
		mgui_parameters.height = min_height;
	}
}

void mgui_add_widget_layer(widget_t *widget)
{
	// Remove the widget from its parent and push to layer list.
	if (widget->parent != NULL) {
		widget_remove_from_parent(widget);
	}
	
	list_push(widgets, widget);
}

void mgui_remove_widget_layer(widget_t *widget)
{
	list_remove(widgets, widget);
}

widget_t *mgui_get_widget_at_position(vec2i_t point)
{
	widget_t *widget = mgui_get_focused_widget(), *hit;

	// Some widgets consume input events as if they were on top when they're focused.
	if (widget != NULL && widget->state & WIDGET_STATE_FOCUS_AS_TOP) {

		hit = widget_get_child_at_position(mgui_get_focused_widget(), point);

		if (hit != NULL) {
			return hit;
		}
	}

	// Then test all layers in reversed add order.
	list_foreach_reverse(widgets, widget) {

		hit = widget_get_child_at_position(widget, point);
		if (hit != NULL) {

			return hit;
		}
	}

	return NULL;
}

void mgui_remove_references_to_object(widget_t *widget)
{
	if (widget == focused_widget) {
		focused_widget = NULL;
	}
	if (widget == dragged_widget) {
		dragged_widget = NULL;
	}
	if (widget == hovered_widget) {
		hovered_widget = NULL;
	}
	if (widget == pressed_widget) {
		pressed_widget = NULL;
	}

	list_remove(widgets, widget);
}

void mgui_add_widget_delayed_render(widget_t *widget)
{
	arr_push(delayed_render_widgets, widget);
}

widget_t *mgui_get_focused_widget(void)
{
	return focused_widget;
}

widget_t *mgui_get_dragged_widget(void)
{
	return dragged_widget;
}

widget_t *mgui_get_hovered_widget(void)
{
	return hovered_widget;
}

widget_t *mgui_get_pressed_widget(void)
{
	return pressed_widget;
}

void mgui_set_focused_widget(widget_t *widget)
{
	if (widget == focused_widget) {
		return;
	}

	// Remove focus from previous widget.
	if (focused_widget != NULL) {

		focused_widget->state &= ~WIDGET_STATE_FOCUSED;

		if (focused_widget->callbacks->on_focused != NULL) {
			focused_widget->callbacks->on_focused(focused_widget, false);
		}
	}

	// Move focus to the new widget.
	focused_widget = widget;

	if (focused_widget != NULL) {

		focused_widget->state |= WIDGET_STATE_FOCUSED;

		if (focused_widget->callbacks->on_focused != NULL) {
			focused_widget->callbacks->on_focused(focused_widget, true);
		}

		// Move the new focused widget (or its grandparent) to the top (i.e. last of the widgets list).
		widget_t *layer = widget_get_grandparent(focused_widget);

		if (layer != NULL &&
			list_contains(widgets, layer)) {

			list_remove(widgets, layer);
			list_push(widgets, layer);
		}
	}
}

void mgui_set_dragged_widget(widget_t *widget)
{
	if (widget == dragged_widget) {
		return;
	}

	if (widget != NULL && (
		widget->state & WIDGET_STATE_PRESSABLE ||
		widget->input_handler != NULL)) {

		// The widget reacts to mouse presses or has a custom input event handler, so it's not
		// a good idea to drag it.
		dragged_widget = NULL;
		return;
	}

	dragged_widget = widget;

	// Drag the widget's root instead of the child widget.
	if (dragged_widget != NULL) {
		dragged_widget = widget_get_draggable_parent(dragged_widget);
	}
}

void mgui_set_hovered_widget(widget_t *widget)
{
	if (widget == hovered_widget) {
		return;
	}

	// Update the state of the previous widget.
	if (hovered_widget != NULL) {

		hovered_widget->state &= ~WIDGET_STATE_HOVERED;

		if (hovered_widget->state & WIDGET_STATE_HOVERABLE) {
			hovered_widget->has_colour_changed = true;
		}

		if (hovered_widget->callbacks->on_hovered != NULL) {
			hovered_widget->callbacks->on_hovered(hovered_widget, false);
		}

		if (hovered_widget->on_hovered != NULL) {
			hovered_widget->on_hovered(hovered_widget, false);
		}
	}

	hovered_widget = widget;

	// Update the state of the new widget.
	if (hovered_widget != NULL) {

		hovered_widget->state |= WIDGET_STATE_HOVERED;

		if (hovered_widget->state & WIDGET_STATE_HOVERABLE) {
			hovered_widget->has_colour_changed = true;
		}

		if (hovered_widget->callbacks->on_hovered != NULL) {
			hovered_widget->callbacks->on_hovered(hovered_widget, true);
		}

		if (hovered_widget->on_hovered != NULL) {
			hovered_widget->on_hovered(hovered_widget, true);
		}
	}
}

void mgui_set_pressed_widget(widget_t *widget, int16_t x, int16_t y)
{
	// Update the state of the previous widget.
	if (pressed_widget != NULL &&
		pressed_widget != widget) {

		pressed_widget->state &= ~WIDGET_STATE_PRESSED;

		if (pressed_widget->state & WIDGET_STATE_PRESSABLE) {
			pressed_widget->has_colour_changed = true;
		}

		if (pressed_widget->on_pressed != NULL) {
			pressed_widget->on_pressed(pressed_widget, false);
		}

		if (pressed_widget->callbacks->on_pressed != NULL) {
			pressed_widget->callbacks->on_pressed(pressed_widget, false, x, y);
		}
	}

	pressed_widget = widget;

	// Update the state of the new widget.
	if (pressed_widget != NULL) {

		// If the widget itself doesn't react to mouse presses but its parent consumes them,
		// relay the press event to the parent instead. This is so we can have e.g. checkboxes with
		// clickable child labels.
		if (pressed_widget->callbacks->on_pressed == NULL &&
			pressed_widget->parent != NULL &&
			pressed_widget->parent->state & WIDGET_STATE_CONSUME_CHILD_PRESSES) {

			pressed_widget = pressed_widget->parent;
		}

		pressed_widget->state |= WIDGET_STATE_PRESSED;

		if (pressed_widget->state & WIDGET_STATE_PRESSABLE) {
			pressed_widget->has_colour_changed = true;
		}

		if (pressed_widget->callbacks->on_pressed != NULL) {
			pressed_widget->callbacks->on_pressed(pressed_widget, true, x, y);
		}

		if (pressed_widget->on_pressed != NULL) {
			pressed_widget->on_pressed(pressed_widget, true);
		}
	}
}

void mgui_convert_to_ui_coordinates(int16_t *x, int16_t *y)
{
	int scaled_x = (int)(*x) * mgui_parameters.width / screen_width;
	int scaled_y = (int)(*y) * mgui_parameters.height / screen_height;

	*x = (int16_t)scaled_x;
	*y = (int16_t)scaled_y;
}
