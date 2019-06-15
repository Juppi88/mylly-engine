#include "dropdown.h"
#include "mgui/text.h"
#include "mgui/widget.h"
#include "scene/sprite.h"
#include "renderer/mesh.h"
#include "renderer/font.h"
#include "core/string.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

static void on_dropdown_process(widget_t *dropdown);
static void on_dropdown_focused(widget_t *dropdown, bool focused);
static void on_dropdown_refresh_vertices(widget_t *dropdown);
static void on_dropdown_pressed(widget_t *dropdown, bool pressed, int16_t x, int16_t y);
static void on_dropdown_input_event(widget_event_t *event);
static void on_dropdown_option_input_event(widget_event_t *event);
static void dropdown_toggle_list(widget_t *dropdown, bool display);
static void dropdown_select_option_label(widget_t *dropdown, widget_t *option);
static widget_t *dropdown_get_option_under_cursor(widget_t *dropdown, int16_t x, int16_t y);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	on_dropdown_process,
	NULL,
	on_dropdown_refresh_vertices,
	on_dropdown_focused,
	NULL,
	on_dropdown_pressed,
	NULL,
	NULL,
	NULL,
};

// -------------------------------------------------------------------------------------------------

widget_t *dropdown_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_DROPDOWN;
	widget->callbacks = &callbacks;
	widget->text = text_create(widget);

	widget->state |= (
		WIDGET_STATE_HOVERABLE |
		WIDGET_STATE_PRESSABLE |
		WIDGET_STATE_HAS_MESH |
		WIDGET_STATE_EXT_MESH |
		WIDGET_STATE_FOCUS_AS_TOP
	);

	// Add a custom input event handler for detecting when options are hovered.
	widget_set_input_handler(widget, on_dropdown_input_event);

	// We'll create a panel widget to serve as the background of the dropdown list.
	widget->dropdown.background = panel_create(widget);
	widget_set_visible(widget->dropdown.background, false);

	// Create another panel to be drawn as the selection background.
	widget->dropdown.selection = panel_create(widget->dropdown.background);
	widget_set_visible(widget->dropdown.selection, false);

	// Create an invisible one-column grid object for containing all the option labels.
	widget->dropdown.grid = grid_create(widget->dropdown.background);
	grid_set_max_items_per_row(widget->dropdown.grid, 1);

	widget_set_anchors(widget->dropdown.grid,
		ANCHOR_MIN, 0,
		ANCHOR_MAX, 0,
		ANCHOR_MIN, 0,
		ANCHOR_MAX, 0
	);

	return widget;
}

void dropdown_set_arrow(widget_t *dropdown, sprite_t *icon)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN || icon == NULL) {
		return;
	}

	// Ensure the icon is on the same sprite sheet as the main widget.
	if (dropdown->sprite != NULL && dropdown->sprite->texture != icon->texture) {

		log_warning("MGUI", "The arrow sprite must be on the same sheet as the dropdown.");
		return;
	}

	dropdown->dropdown.arrow = icon;
	dropdown->has_colour_changed = true;
}

void dropdown_set_arrow_colour(widget_t *dropdown, colour_t colour)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN) {
		return;
	}

	dropdown->dropdown.arrow_colour = colour;
	dropdown->has_colour_changed = true;
}

void dropdown_set_background(widget_t *dropdown, sprite_t *background)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN || background == NULL) {
		return;
	}

	widget_set_sprite(dropdown->dropdown.background, background);
}

void dropdown_set_background_colour(widget_t *dropdown, colour_t colour)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN) {
		return;
	}

	widget_set_colour(dropdown->dropdown.background, colour);
}

void dropdown_set_selection(widget_t *dropdown, sprite_t *background)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN || background == NULL) {
		return;
	}

	widget_set_sprite(dropdown->dropdown.selection, background);
}

void dropdown_set_selection_colour(widget_t *dropdown, colour_t colour)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN) {
		return;
	}

	widget_set_colour(dropdown->dropdown.selection, colour);
}

void dropdown_add_option(widget_t *dropdown, const char *option, void *data)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN ||
		string_is_null_or_empty(option)) {

		return;
	}

	// Create a label as the child of the option grid.
	widget_t *option_label = label_create(dropdown->dropdown.grid);

	widget_set_text_s(option_label, option);
	widget_set_user_context(option_label, data);

	widget_set_text_font(option_label, dropdown->text->font);
	widget_set_text_colour(option_label, dropdown->text->colour);
	widget_set_text_alignment(option_label, ALIGNMENT_LEFT);

	// Add a custom input event handler for detecting when the option is clicked.
	widget_set_input_handler(option_label, on_dropdown_option_input_event);

	// Reposition the option grid.
	grid_reposition(dropdown->dropdown.grid);

	// Select the first option by default.
	if (dropdown->dropdown.grid->num_children == 1) {
		dropdown_select_option_label(dropdown, option_label);
	}
}

void dropdown_clear_options(widget_t *dropdown)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN) {
		return;
	}

	// Deselect the current option.
	dropdown_select_option_label(dropdown, NULL);

	// Destroy all the options in this dropdown.
	widget_t *option, *tmp;

	list_foreach_safe(dropdown->dropdown.grid->children, option, tmp) {
		widget_destroy(option);
	}

	grid_reposition(dropdown->dropdown.grid);
}

void dropdown_set_selected_handler(widget_t *dropdown, on_dropdown_item_selected_t handler)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN) {
		return;
	}

	dropdown->dropdown.on_selected = handler;
}

const char *dropdown_get_selected_option(widget_t *dropdown)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN ||
		dropdown->dropdown.selected_option == NULL) {
		return NULL;
	}

	return dropdown->dropdown.selected_option->text->buffer;
}

void *dropdown_get_selected_option_data(widget_t *dropdown)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN ||
		dropdown->dropdown.selected_option == NULL) {
		return NULL;
	}

	return dropdown->dropdown.selected_option->user_context;
}

void dropdown_select_option(widget_t *dropdown, const char *option)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN) {
		return;
	}

	widget_t *option_label;
	list_foreach(dropdown->dropdown.grid->children, option_label) {

		if (string_equals(option, option_label->text->buffer)) {

			dropdown_select_option_label(dropdown, option_label);
			return;
		}
	}
}

void dropdown_select_option_by_data(widget_t *dropdown, void *data)
{
	if (dropdown == NULL || dropdown->type != WIDGET_TYPE_DROPDOWN) {
		return;
	}

	widget_t *option_label;
	list_foreach(dropdown->dropdown.grid->children, option_label) {

		if (data == option_label->user_context) {

			dropdown_select_option_label(dropdown, option_label);
			return;
		}
	}
}

static void on_dropdown_process(widget_t *dropdown)
{
	if (dropdown == NULL) {
		return;
	}

	// Ensure all options have the same text settings as the parent.
	widget_t *option;

	list_foreach(dropdown->dropdown.grid->children, option) {

		if (option->text->font != dropdown->text->font) {
			widget_set_text_font(option, dropdown->text->font);
		}

		if (col_equals(option->text->colour, dropdown->text->colour)) {
			widget_set_text_colour(option, dropdown->text->colour);
		}
	}

	// Calculate a height for the background sprite.
	const int16_t PADDING = 10;

	int16_t item_width = dropdown->dropdown.grid->size.x;
	int16_t item_height = (dropdown->text->font != 0 ? dropdown->text->font->size + PADDING : 20);

	dropdown->dropdown.list_height = dropdown->dropdown.grid->num_children * item_height;

	// Ensure minimum height if the list has no options.
	if (dropdown->dropdown.list_height < PADDING) {
		dropdown->dropdown.list_height = PADDING;
	}

	// Adjust the size of an option label within the grid.
	grid_set_item_size(dropdown->dropdown.grid, vec2i(item_width, item_height));
}

static void on_dropdown_focused(widget_t *dropdown, bool focused)
{
	if (dropdown == NULL) {
		return;
	}

	// When the dropdown loses focus, close the option list.
	if (!focused && dropdown->dropdown.is_opened) {
		dropdown_toggle_list(dropdown, false);
	}
}

static void on_dropdown_refresh_vertices(widget_t *dropdown)
{
	if (dropdown == NULL || dropdown->dropdown.arrow == NULL) {
		return;
	}

	vertex_ui_t *vertices = dropdown->mesh->ui_vertices;
	sprite_t *sprite = dropdown->dropdown.arrow;
	colour_t col = dropdown->dropdown.arrow_colour;

	vec2_t uv1 = sprite->uv1;
	vec2_t uv2 = sprite->uv2;
	vec2_t extents = vec2_multiply(sprite->size, 0.5f);

	float right = vertices[2].pos.x;
	float centre = (vertices[1].pos.y + vertices[0].pos.y) / 2;
	float width = 2 * extents.x;

	vertices[16] = vertex_ui(vec2(right - width, centre - extents.y), vec2(uv1.x, uv1.y), col);
	vertices[17] = vertex_ui(vec2(right - width, centre + extents.y), vec2(uv1.x, uv2.y), col);
	vertices[18] = vertex_ui(vec2(right, centre - extents.y), vec2(uv2.x, uv1.y), col);
	vertices[19] = vertex_ui(vec2(right, centre + extents.y), vec2(uv2.x, uv2.y), col);

	// Update background anchors.
	widget_set_anchors(dropdown->dropdown.background,
		ANCHOR_MIN, 0,
		ANCHOR_MAX, 0,
		ANCHOR_MAX, 1,
		ANCHOR_MAX, dropdown->dropdown.list_height + 1
	);
}

static void on_dropdown_pressed(widget_t *dropdown, bool pressed, int16_t x, int16_t y)
{
	UNUSED(x);
	UNUSED(y);
	
	if (dropdown == NULL) {
		return;
	}

	if (!pressed) {
		dropdown_toggle_list(dropdown, !dropdown->dropdown.is_opened);
	}
}

static void on_dropdown_input_event(widget_event_t *event)
{
	if (!event->widget->dropdown.is_opened) {
		return;
	}

	// Handle mouse hover.
	vec2i_t mouse_pos = vec2i(event->mouse.x, event->mouse.y);

	if (event->type == INPUT_MOUSE_MOVE) {

		widget_t *option = dropdown_get_option_under_cursor(event->widget, mouse_pos.x, mouse_pos.y);

		if (option != NULL &&
			option != event->widget->dropdown.hovered_option) {

			event->widget->dropdown.hovered_option = option;

			// Display and reposition the selection background behind the selected option.
			widget_set_visible(event->widget->dropdown.selection, true);

			widget_set_position(event->widget->dropdown.selection, widget_get_position(option));
			widget_set_size(event->widget->dropdown.selection, widget_get_size(option));
		}
	}
}

static void on_dropdown_option_input_event(widget_event_t *event)
{
	// Handle mouse hover.
	if (event->type == INPUT_MOUSE_BUTTON_DOWN) {

		widget_t *dropdown = event->widget->parent->parent->parent; // This is a bit ugly
		dropdown_select_option_label(dropdown, event->widget);
	}
}

static void dropdown_toggle_list(widget_t *dropdown, bool display)
{
	widget_set_visible(dropdown->dropdown.background, display);

	dropdown->dropdown.is_opened = display;

	if (!display) {

		// Hide the dropdown list. The dropdown no longer needs to be rendered late.
		dropdown->state &= ~WIDGET_STATE_DELAYED_RENDER;
		return;
	}

	// Hide the selection background until an option is hovered.
	dropdown->dropdown.hovered_option = NULL;
	widget_set_visible(dropdown->dropdown.selection, false);

	// Render the dropdown last so the opened list covers other widgets.
	dropdown->state |= WIDGET_STATE_DELAYED_RENDER;
}

static void dropdown_select_option_label(widget_t *dropdown, widget_t *option)
{
	if (dropdown == NULL) {
		return;
	}

	if (option != NULL) {

		// Show the name of the selected option on the dropdown.
		widget_set_text_s(dropdown, option->text->buffer);

		// Store selected option widget for future access.
		dropdown->dropdown.selected_option = option;

		// If the user has provided a callback method for selection, fire it.
		if (dropdown->dropdown.on_selected != NULL) {
			dropdown->dropdown.on_selected(dropdown, option->text->buffer, option->user_context);
		}
	}
	else {
		widget_set_text_s(dropdown, "");
		dropdown->dropdown.selected_option = NULL;
	}
}

static widget_t *dropdown_get_option_under_cursor(widget_t *dropdown, int16_t x, int16_t y)
{
	widget_t *option;

	list_foreach(dropdown->dropdown.grid->children, option) {

		if (widget_is_point_inside(option, vec2i(x, y))) {
			return option;
		}
	}

	return NULL;
}
