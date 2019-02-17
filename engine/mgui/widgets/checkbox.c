#include "checkbox.h"
#include "mgui/text.h"
#include "mgui/widget.h"
#include "scene/sprite.h"
#include "renderer/mesh.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

static void on_checkbox_refresh_vertices(widget_t *checkbox);
static void on_checkbox_pressed(widget_t *checkbox, bool pressed);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	NULL,
	on_checkbox_refresh_vertices,
	NULL,
	NULL,
	on_checkbox_pressed,
};

// -------------------------------------------------------------------------------------------------

widget_t *checkbox_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_CHECKBOX;
	widget->callbacks = &callbacks;

	widget->state |= (
		WIDGET_STATE_HOVERABLE |
		WIDGET_STATE_PRESSABLE |
		WIDGET_STATE_CONSUME_CHILD_PRESSES | // Clicking child (e.g. label) clicks the checkbox
		WIDGET_STATE_HAS_MESH |
		WIDGET_STATE_EXT_MESH
	);

	return widget;
}

void checkbox_set_toggled(widget_t *checkbox, bool toggled)
{
	if (checkbox == NULL || checkbox->type != WIDGET_TYPE_CHECKBOX) {
		return;
	}

	// Don't update if there would be no change to the status.
	if (toggled == checkbox->checkbox.is_toggled) {
		return;
	}

	// Set toggled status.
	checkbox->checkbox.is_toggled = toggled;
	checkbox->has_colour_changed = true;

	// Call toggled status callback.
	if (checkbox->checkbox.on_toggled != NULL) {
		checkbox->checkbox.on_toggled(checkbox->checkbox.on_toggled_context);
	}
}

void checkbox_set_icon(widget_t *checkbox, sprite_t *icon)
{
	if (checkbox == NULL || checkbox->type != WIDGET_TYPE_CHECKBOX || icon == NULL) {
		return;
	}

	// Ensure the icon is on the same sprite sheet as the main widget.
	if (checkbox->sprite != NULL && checkbox->sprite->texture != icon->texture) {

		log_warning("MGUI", "The icon of the checkbox must be on the same sheet as the checkbox.");
		return;
	}

	checkbox->checkbox.icon = icon;
	checkbox->has_colour_changed = true;
}

void checkbox_set_icon_colour(widget_t *checkbox, colour_t colour)
{
	if (checkbox == NULL || checkbox->type != WIDGET_TYPE_CHECKBOX) {
		return;
	}

	checkbox->checkbox.icon_colour = colour;
	checkbox->has_colour_changed = true;
}

void checkbox_set_toggled_handler(widget_t *checkbox, on_checkbox_toggled_t handler, void *context)
{
	if (checkbox == NULL || checkbox->type != WIDGET_TYPE_CHECKBOX) {
		return;
	}

	checkbox->checkbox.on_toggled = handler;
	checkbox->checkbox.on_toggled_context = context;
}

static void on_checkbox_refresh_vertices(widget_t *checkbox)
{
	if (checkbox == NULL || checkbox->checkbox.icon == NULL) {
		return;
	}

	vertex_ui_t *vertices = checkbox->mesh->ui_vertices;
	sprite_t *sprite = checkbox->checkbox.icon;
	colour_t colour = (checkbox->checkbox.is_toggled ?
	                   checkbox->checkbox.icon_colour :
	                   COL_TRANSPARENT);

	vec2_t uv1 = sprite->uv1;
	vec2_t uv2 = sprite->uv2;

	// Copy the vertex coordinates of the centre quad, change texture coordinates and colour.
	vertices[16] = vertex_ui(vertices[4].pos, vec2(uv1.x, uv2.y), colour);
	vertices[17] = vertex_ui(vertices[5].pos, vec2(uv1.x, uv1.y), colour);
	vertices[18] = vertex_ui(vertices[6].pos, vec2(uv2.x, uv2.y), colour);
	vertices[19] = vertex_ui(vertices[7].pos, vec2(uv2.x, uv1.y), colour);
}

static void on_checkbox_pressed(widget_t *checkbox, bool pressed)
{
	if (checkbox == NULL) {
		return;
	}

	if (!pressed) {
		
		// Toggle checkbox status.
		checkbox->checkbox.is_toggled = !checkbox->checkbox.is_toggled;
		checkbox->has_colour_changed = true;

		// Call toggled status callback.
		if (checkbox->checkbox.on_toggled != NULL) {
			checkbox->checkbox.on_toggled(checkbox->checkbox.on_toggled_context);
		}
	}
}
