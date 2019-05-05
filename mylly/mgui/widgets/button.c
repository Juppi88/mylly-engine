#include "button.h"
#include "mgui/text.h"
#include "mgui/widget.h"

// -------------------------------------------------------------------------------------------------

const uint32_t BUTTON_COLOUR_LERP_TIME = 150;

// -------------------------------------------------------------------------------------------------

static void on_button_process(widget_t *button);
static void on_button_hovered(widget_t *button, bool hovered);
static void on_button_pressed(widget_t *button, bool pressed);
static void button_interpolate_colour(widget_t *button, colour_t from, colour_t to);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	on_button_process,
	NULL,
	NULL,
	NULL,
	on_button_hovered,
	on_button_pressed,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

widget_t *button_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_BUTTON;
	widget->callbacks = &callbacks;
	widget->state |= (WIDGET_STATE_HAS_MESH | WIDGET_STATE_HOVERABLE | WIDGET_STATE_PRESSABLE);
	widget->text = text_create(widget);

	widget->button.normal_colour = COL_WHITE;
	widget->button.hovered_colour = COL_WHITE;
	widget->button.pressed_colour = COL_WHITE;

	return widget;
}

void button_set_colours(widget_t *button, colour_t normal, colour_t hovered, colour_t pressed)
{
	if (button == NULL || button->type != WIDGET_TYPE_BUTTON) {
		return;
	}

	button->button.normal_colour = normal;
	button->button.hovered_colour = hovered;
	button->button.pressed_colour = pressed;

	// Apply the new colours immediately.
	if (button->state & WIDGET_STATE_PRESSED) {
		widget_set_colour(button, pressed);
	}
	else if (button->state & WIDGET_STATE_HOVERED) {
		widget_set_colour(button, hovered);
	}
	else {
		widget_set_colour(button, normal);
	}
}

void button_set_clicked_handler(widget_t *button, on_button_clicked_t handler)
{
	if (button == NULL || button->type != WIDGET_TYPE_BUTTON) {
		return;
	}

	button->button.on_clicked = handler;
}

static void on_button_process(widget_t *button)
{
	if (button == NULL) {
		return;
	}

	if (!timer_has_started(&button->button.colour_timer)) {
		return;
	}

	// Check whether the colour interpolation has finished.
	if (timer_has_expired(&button->button.colour_timer, BUTTON_COLOUR_LERP_TIME)) {

		widget_set_colour(button, button->button.colour_to);
		return;
	}

	// Update the colour of the button.
	float t = timer_get_factor(&button->button.colour_timer, BUTTON_COLOUR_LERP_TIME);
	colour_t colour = col_lerp(button->button.colour_from, button->button.colour_to, t);

	widget_set_colour(button, colour);
}

static void on_button_hovered(widget_t *button, bool hovered)
{
	if (button == NULL) {
		return;
	}

	if (hovered) {
		button_interpolate_colour(button, button->colour, button->button.hovered_colour);
	}
	else {
		button_interpolate_colour(button, button->colour, button->button.normal_colour);
	}
}

static void on_button_pressed(widget_t *button, bool pressed)
{
	if (button == NULL) {
		return;
	}

	if (pressed) {
		button_interpolate_colour(button, button->colour, button->button.pressed_colour);
	}
	else if (button->state & WIDGET_STATE_HOVERED) {
		
		button_interpolate_colour(button, button->colour, button->button.hovered_colour);

		// Buton was released but is still hovered -> call click handler.
		if (button->button.on_clicked != NULL) {
			button->button.on_clicked(button);
		}
	}
	else {
		button_interpolate_colour(button, button->colour, button->button.normal_colour);
	}
}

static void button_interpolate_colour(widget_t *button, colour_t from, colour_t to)
{
	timer_reset(&button->button.colour_timer, BUTTON_COLOUR_LERP_TIME);

	button->button.colour_from = from;
	button->button.colour_to = to;
}
