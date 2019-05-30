#include "mgui/widget.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"
#include "renderer/shader.h"
#include "resources/resources.h"
#include "scene/sprite.h"
#include "math/math.h"

// -------------------------------------------------------------------------------------------------

static void colourpicker_handle_click(widget_t *picker, int16_t click_x, int16_t click_y);

static void on_colourpicker_refresh_vertices(widget_t *picker);
static void on_colourpicker_pressed(widget_t *picker, bool pressed, int16_t x, int16_t y);
static void on_colourpicker_dragged(widget_t *picker, int16_t x, int16_t y);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	NULL,
	on_colourpicker_refresh_vertices,
	NULL,
	NULL,
	on_colourpicker_pressed,
	on_colourpicker_dragged,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

static const char *PICKER_TEXTURE_NAME = "__COLOUR_PICKER_TEXTURE";
static const uint16_t PICKER_TEXTURE_WIDTH = 512;
static const uint16_t PICKER_TEXTURE_HEIGHT = 512;

// -------------------------------------------------------------------------------------------------

static texture_t *picker_texture;
static sprite_t *picker_sprite;

// -------------------------------------------------------------------------------------------------

widget_t *colourpicker_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_COLOURPICKER;
	widget->callbacks = &callbacks;

	widget->state |= (
		WIDGET_STATE_HAS_MESH |
		WIDGET_STATE_DRAGGABLE
	);

	// By default white is selected.
	widget->colour_picker.colour = COL_WHITE;

	// Use the generated colour spectrum texture as the widget's sprite.
	widget_set_sprite(widget, picker_sprite);

	// Render the colour picker with a custom shader.
	widget_set_custom_shader(widget, res_get_shader("default-colour-picker"));
	colourpicker_set_brightness_alpha(widget, 1.0f, 1.0f);

	return widget;
}

colour_t colourpicker_get_colour(widget_t *picker)
{
	if (picker == NULL || picker->type != WIDGET_TYPE_COLOURPICKER) {
		return COL_TRANSPARENT;
	}

	return picker->colour_picker.colour;
}

void colourpicker_set_colour(widget_t *picker, colour_t colour)
{
	if (picker == NULL || picker->type != WIDGET_TYPE_COLOURPICKER) {
		return;
	}

	picker->colour_picker.colour = colour;
}

void colourpicker_set_brightness_alpha(widget_t *picker, float brightness, float alpha)
{
	if (picker == NULL || picker->type != WIDGET_TYPE_COLOURPICKER) {
		return;
	}

	picker->colour_picker.brightness = brightness;
	picker->colour_picker.alpha = (uint8_t)(255 * alpha);

	shader_set_uniform_float(picker->custom_shader, "Brightness", brightness);
	shader_set_uniform_float(picker->custom_shader, "Alpha", alpha);
}

void colourpicker_set_selected_handler(widget_t *picker, on_colour_selected_t handler)
{
	if (picker == NULL || picker->type != WIDGET_TYPE_COLOURPICKER) {
		return;
	}

	picker->colour_picker.on_selected = handler;
}

void colourpicker_create_texture(void)
{
	if (picker_texture != NULL ||
		picker_sprite != NULL) {
		return;
	}

	// Allocate an array for the colour picker texture and populate it.
	// Note that this memory block is handed over to the texture and freed when the texture
	// is destroyed.
	uint32_t *bitmap = mem_alloc_fast(sizeof(uint32_t) * PICKER_TEXTURE_WIDTH * PICKER_TEXTURE_HEIGHT);

	for (uint32_t j = 0; j < PICKER_TEXTURE_HEIGHT; j++) {

		for (uint32_t i = 0; i < PICKER_TEXTURE_WIDTH; i++) {

			float hue = (float)i / PICKER_TEXTURE_WIDTH;
			float saturation = 1.0f * j / PICKER_TEXTURE_HEIGHT;
			float value = 1.0f;

			colour_t colour = col_from_hsv(hue, saturation, value);

			*(colour_t *)&bitmap[j * PICKER_TEXTURE_WIDTH + i] = colour;
		}
	}

	// Upload the texture to the GPU.
	picker_texture = texture_create(PICKER_TEXTURE_NAME, NULL);

	texture_load_bitmap(picker_texture, (uint8_t *)bitmap, PICKER_TEXTURE_WIDTH, PICKER_TEXTURE_HEIGHT,
	                    TEX_FORMAT_RGBA, TEX_FILTER_BILINEAR);

	// Create a sprite which we can use as the the UI widget's background.
	vec2_t size = vec2(PICKER_TEXTURE_WIDTH, PICKER_TEXTURE_HEIGHT);

	picker_sprite = sprite_create(picker_texture, NULL);
	sprite_set(picker_sprite, picker_texture, vec2_zero(), size, vec2_multiply(size, 0.5f), 1.0f);
}

void colourpicker_destroy_texture(void)
{
	if (picker_sprite != NULL) {

		sprite_destroy(picker_sprite);
		picker_sprite = NULL;
	}

	if (picker_texture != NULL) {

		texture_destroy(picker_texture);
		picker_texture = NULL;
	}
}

static void on_colourpicker_refresh_vertices(widget_t *picker)
{
	if (picker == NULL || picker->custom_shader == NULL) {
		return;
	}

	vec2i_t position = widget_get_world_position(picker);

	shader_set_uniform_float(picker->custom_shader, "WidgetPosX", position.x);
	shader_set_uniform_float(picker->custom_shader, "WidgetPosY", mgui_parameters.height - position.y);
}

static void colourpicker_handle_click(widget_t *picker, int16_t click_x, int16_t click_y)
{
	vec2i_t position = widget_get_world_position(picker);
	vec2i_t size = widget_get_size(picker);

	int16_t x = click_x - position.x;
	int16_t y = click_y - position.y;

	x = CLAMP(x, 0, size.x);
	y = CLAMP(y, 0, size.y);

	float hue = (float)x / size.x;
	float saturation = 1.0f - (float)y / size.y;
	float value = picker->colour_picker.brightness;

	colour_t colour = col_from_hsv(hue, saturation, value);
	colour.a = picker->colour_picker.alpha;
	
	if (picker->colour_picker.on_selected != NULL) {
		picker->colour_picker.on_selected(picker, colour);
	}
}

static void on_colourpicker_pressed(widget_t *picker, bool pressed, int16_t x, int16_t y)
{
	UNUSED(pressed);
	
	if (picker == NULL) {
		return;
	}

	colourpicker_handle_click(picker, x, y);
}

static void on_colourpicker_dragged(widget_t *picker, int16_t x, int16_t y)
{
	if (picker == NULL) {
		return;
	}

	colourpicker_handle_click(picker, x, y);
}
