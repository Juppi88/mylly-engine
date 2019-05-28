#include "mgui/widget.h"
#include "renderer/mesh.h"
#include "scene/sprite.h"
#include "math/math.h"

// -------------------------------------------------------------------------------------------------

static float slider_get_closest_value(widget_t *slider, float value);
static void slider_handle_click(widget_t *slider, int16_t click_x);

static void on_slider_refresh_vertices(widget_t *slider);
static void on_slider_pressed(widget_t *slider, bool pressed, int16_t x, int16_t y);
static void on_slider_dragged(widget_t *slider, int16_t x, int16_t y);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	NULL,
	on_slider_refresh_vertices,
	NULL,
	NULL,
	on_slider_pressed,
	on_slider_dragged,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

// Horizontal pad for the movement of the knob.
// TODO: Don't use these horrible magic constants.
const int16_t PAD = 5;

// -------------------------------------------------------------------------------------------------

widget_t *slider_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_SLIDER;
	widget->callbacks = &callbacks;

	widget->state |= (
		WIDGET_STATE_HAS_MESH |
		WIDGET_STATE_EXT_MESH |
		WIDGET_STATE_DRAGGABLE
	);

	// Initialize default values.
	widget->slider.value = 0;
	widget->slider.min_value = 0;
	widget->slider.max_value = 100;
	widget->slider.num_steps = 101; // steps [0...100]

	widget->slider.knob_colour = COL_WHITE;
	widget->slider.on_changed = NULL;

	return widget;
}

float slider_get_value(widget_t *slider)
{
	if (slider == NULL || slider->type != WIDGET_TYPE_SLIDER) {
		return 0;
	}

	return slider->slider.value;
}

void slider_set_value(widget_t *slider, float value)
{
	if (slider == NULL || slider->type != WIDGET_TYPE_SLIDER) {
		return;
	}

	// Ensure the new value is within accepted limits.
	value = CLAMP(value, slider->slider.min_value, slider->slider.max_value);
	value = slider_get_closest_value(slider, value);

	// Fire the change callback.
	if (value != slider->slider.value) {

		slider->slider.value = value;
		slider->has_colour_changed = true;

		if (slider->slider.on_changed != NULL) {
			slider->slider.on_changed(slider);
			}
	}
}

void slider_set_range(widget_t *slider, float min_value, float max_value)
{
	if (slider == NULL || slider->type != WIDGET_TYPE_SLIDER) {
		return;
	}

	slider->slider.min_value = min_value;
	slider->slider.max_value = max_value;

	// Ensure the value of the slider is still within accepted limits.
	float value = slider->slider.value;
	value = CLAMP(value, min_value, max_value);
	value = slider_get_closest_value(slider, value);

	// Fire the change callback.
	if (value != slider->slider.value) {

		slider->slider.value = value;
		slider->has_colour_changed = true;

		if (slider->slider.on_changed != NULL) {
			slider->slider.on_changed(slider);
		}
	}
}

void slider_set_steps(widget_t *slider, int num_steps)
{
	if (slider == NULL || slider->type != WIDGET_TYPE_SLIDER) {
		return;
	}

	// Ensure at least 2 steps.
	if (num_steps < 2) {
		num_steps = 2;
	}

	slider->slider.num_steps = num_steps;

	// Make the value of the slider match the number of steps.
	float value = slider_get_closest_value(slider, slider->slider.value);

	if (value != slider->slider.value) {

		slider->slider.value = value;
		slider->has_colour_changed = true;

		if (slider->slider.on_changed != NULL) {
			slider->slider.on_changed(slider);
		}
	}
}

void slider_set_knob_sprite(widget_t *slider, sprite_t *sprite)
{
	if (slider == NULL || slider->type != WIDGET_TYPE_SLIDER) {
		return;
	}

	slider->slider.knob = sprite;
}

void slider_set_knob_colour(widget_t *slider, colour_t colour)
{
	if (slider == NULL || slider->type != WIDGET_TYPE_SLIDER) {
		return;
	}

	slider->slider.knob_colour = colour;
}

void slider_set_value_changed_handler(widget_t *slider, on_slider_value_changed_t handler)
{
	if (slider == NULL || slider->type != WIDGET_TYPE_SLIDER) {
		return;
	}

	slider->slider.on_changed = handler;
}

static float slider_get_closest_value(widget_t *slider, float value)
{
	// Assume the value is already within [min...max] range.
	// Convert the value to [0...num_steps-1] range.
	float t = (value - slider->slider.min_value) /
	          (slider->slider.max_value - slider->slider.min_value);

	float step = t * (slider->slider.num_steps - 1);

	// Round the value to the closest step and convert it back to [min...max] range.
	step = roundf(step);
	t = step / (slider->slider.num_steps - 1);

	return slider->slider.min_value + t * (slider->slider.max_value - slider->slider.min_value);
}

static void slider_handle_click(widget_t *slider, int16_t click_x)
{
	vec2i_t position = widget_get_world_position(slider);
	vec2i_t size = widget_get_size(slider);

	// Calculate the relative position of the clicked position (in 0...1 range).
	int16_t x = click_x - (position.x + PAD);

	float t = (float)x / (size.x - 2 * PAD);
	t = CLAMP01(t);

	// Calculate absolute value and update it.
	float val = slider->slider.min_value + t * (slider->slider.max_value - slider->slider.min_value);
	slider_set_value(slider, val);
}

static void on_slider_pressed(widget_t *slider, bool pressed, int16_t x, int16_t y)
{
	UNUSED(y);
	UNUSED(pressed);
	
	if (slider == NULL) {
		return;
	}

	slider_handle_click(slider, x);
}

static void on_slider_dragged(widget_t *slider, int16_t x, int16_t y)
{
	UNUSED(y);
	
	if (slider == NULL) {
		return;
	}

	slider_handle_click(slider, x);
}

static void on_slider_refresh_vertices(widget_t *slider)
{
	if (slider == NULL || slider->slider.knob == NULL) {
		return;
	}

	vertex_ui_t *vertices = slider->mesh->ui_vertices;
	sprite_t *sprite = slider->slider.knob;
	colour_t colour = slider->slider.knob_colour;

	vec2_t uv1 = sprite->uv1;
	vec2_t uv2 = sprite->uv2;

	// Calculate the position of the knob.
	float t = (slider->slider.value - slider->slider.min_value) /
	          (slider->slider.max_value - slider->slider.min_value);

	t = CLAMP01(t);

	vec2i_t slider_position = widget_get_world_position(slider);
	vec2i_t slider_size = widget_get_size(slider);
	vec2_t ext = vec2_multiply(sprite->size, 0.5f); // The extents of the knob sprite

	int16_t x = slider_position.x + (int16_t)(t * (slider_size.x - 2 * PAD)) + PAD;
	int16_t y = slider_position.y + ext.y / 2;
	float bottom = mgui_parameters.height;

	// Calculate vertex positions for the knob quad.
	vertices[16] = vertex_ui(vec2(x - ext.x, bottom - (y - ext.y)), vec2(uv1.x, uv1.y), colour);
	vertices[17] = vertex_ui(vec2(x - ext.x, bottom - (y + ext.y)), vec2(uv1.x, uv2.y), colour);
	vertices[18] = vertex_ui(vec2(x + ext.x, bottom - (y - ext.y)), vec2(uv2.x, uv1.y), colour);
	vertices[19] = vertex_ui(vec2(x + ext.x, bottom - (y + ext.y)), vec2(uv2.x, uv2.y), colour);
}
