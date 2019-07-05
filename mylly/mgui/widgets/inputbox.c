#include "mgui/text.h"
#include "mgui/widget.h"
#include "scene/sprite.h"
#include "renderer/mesh.h"
#include "renderer/font.h"
#include "io/log.h"
#include "core/string.h"
#include "math/math.h"

// -------------------------------------------------------------------------------------------------

#define MAX_INPUT_LENGTH 1024

// -------------------------------------------------------------------------------------------------

static void inputbox_move_cursor(widget_t *inputbox, bool left);
static void inputbox_remove_character(widget_t *inputbox, uint32_t position);
static void inputbox_switch_focus(widget_t *inputbox, bool forward);
static void inputbox_update_cursor_position(widget_t *inputbox);

static void on_inputbox_focused(widget_t *inputbox, bool focused);
static void on_inputbox_refresh_vertices(widget_t *inputbox);
static void on_inputbox_pressed(widget_t *inputbox, bool pressed, int16_t x, int16_t y);
static void on_inputbox_dragged(widget_t *inputbox, int16_t x, int16_t y);
static bool on_inputbox_key_pressed(widget_t *inputbox, uint32_t key, bool pressed);
static bool on_inputbox_character_injected(widget_t *inputbox, uint32_t c);

// -------------------------------------------------------------------------------------------------

static widget_callbacks_t callbacks = {
	NULL,
	NULL,
	NULL,
	on_inputbox_refresh_vertices,
	on_inputbox_focused,
	NULL,
	on_inputbox_pressed,
	on_inputbox_dragged,
	on_inputbox_key_pressed,
	on_inputbox_character_injected,
};

// -------------------------------------------------------------------------------------------------

widget_t *inputbox_create(widget_t *parent)
{
	// Create the widget.
	widget_t *widget = widget_create(parent);

	// Initialize type specific data.
	widget->type = WIDGET_TYPE_INPUTBOX;
	widget->callbacks = &callbacks;
	widget->text = text_create(widget);

	widget->state |= (
		WIDGET_STATE_HAS_MESH |
		WIDGET_STATE_EXT_MESH |
		WIDGET_STATE_DRAGGABLE
	);

	widget_set_text_alignment(widget, ALIGNMENT_LEFT);

	widget->inputbox.cursor_width = 1;

	return widget;
}

void inputbox_set_cursor_sprite(widget_t *inputbox, sprite_t *cursor, uint8_t width)
{
	if (inputbox == NULL || inputbox->type != WIDGET_TYPE_INPUTBOX || cursor == NULL) {
		return;
	}

	inputbox->inputbox.cursor = cursor;
	inputbox->inputbox.cursor_width = width;

	// Refresh the vertices for the cursor.
	inputbox->has_colour_changed = true;
}

bool inputbox_has_selected_text(widget_t *inputbox)
{
	if (inputbox == NULL || inputbox->type != WIDGET_TYPE_INPUTBOX) {
		return false;
	}

	return (inputbox->inputbox.cursor_start != inputbox->inputbox.cursor_end);
}

static void inputbox_move_cursor(widget_t *inputbox, bool left)
{
	if (input_is_key_down(MKEY_SHIFT)) {

		// Shift is pressed, move selection left or right.
		if (left && inputbox->inputbox.cursor_end > 0) {
			inputbox->inputbox.cursor_end--;
		}
		else if (!left && inputbox->inputbox.cursor_end < inputbox->text->buffer_length) {
			inputbox->inputbox.cursor_end++;
		}
	}
	else {

		// Shift is not pressed, move the cursor left or right.
		uint32_t cursor_pos;

		if (left) {
			cursor_pos = MIN(inputbox->inputbox.cursor_start, inputbox->inputbox.cursor_end);
		}
		else {
			cursor_pos = MAX(inputbox->inputbox.cursor_start, inputbox->inputbox.cursor_end);
		}

		if (inputbox_has_selected_text(inputbox)) {

			// User has selected text -> deselect everything.
			inputbox->inputbox.cursor_start = cursor_pos;
			inputbox->inputbox.cursor_end = cursor_pos;
		}
		else if (left && cursor_pos > 0) {

			// Move cursor to the left.
			inputbox->inputbox.cursor_start = cursor_pos - 1;
			inputbox->inputbox.cursor_end = cursor_pos - 1;
		}
		else if (!left && cursor_pos < inputbox->text->buffer_length) {

			// Move cursor to the right.
			inputbox->inputbox.cursor_start = cursor_pos + 1;
			inputbox->inputbox.cursor_end = cursor_pos + 1;
		}
	}

	// Refresh cursor position.
	inputbox_update_cursor_position(inputbox);
	inputbox->has_colour_changed = true;
}

static void inputbox_remove_character(widget_t *inputbox, uint32_t position)
{
	// Nothing to remove if the cursor is at either end of the text.
	if (!inputbox_has_selected_text(inputbox) &&
		(position == 0 || position > inputbox->text->buffer_length)) {
		return;
	}

	uint32_t cursor_start;
	uint32_t cursor_end;

	if (inputbox->inputbox.cursor_start < inputbox->inputbox.cursor_end) {
		cursor_start = inputbox->inputbox.cursor_start;
		cursor_end = inputbox->inputbox.cursor_end;
	}
	else {
		cursor_start = inputbox->inputbox.cursor_end;
		cursor_end = inputbox->inputbox.cursor_start;
	}

	char tmp[MAX_INPUT_LENGTH + 1];
	char *s = inputbox->text->buffer;
	char *d = tmp;
	size_t i = 0;

	if (inputbox_has_selected_text(inputbox)) {

		// Remove current selection and nothing else.
		while (*s && i < MAX_INPUT_LENGTH) {

			if (i++ == cursor_start) {

				// Skip the selection.
				s += (cursor_end - cursor_start);
				continue;
			}
			else {

				// Copy everything else.
				*d++ = *s++;
			}
		}

		// Move cursor to the beginning of the deleted selection.
		inputbox->inputbox.cursor_start = cursor_start;
		inputbox->inputbox.cursor_end = cursor_start;
	}
	else {

		// Copy all the characters from the existing text buffer to the temporary buffer - except
		// for the one to be removed.
		while (*s && i < MAX_INPUT_LENGTH) {

			if (i++ == position - 1) {

				++s;
				continue;
			}

			*d++ = *s++;
		}

		// Move cursor position back by one.
		inputbox->inputbox.cursor_start = position - 1;
		inputbox->inputbox.cursor_end = position - 1;
	}

	// Terminate the temporary buffer.
	*d = 0;

	// Copy the updated buffer into the text object.
	widget_set_text_s(inputbox, tmp);

	// Refresh cursor position.
	inputbox_update_cursor_position(inputbox);

	inputbox->has_colour_changed = true;
}

static void inputbox_switch_focus(widget_t *inputbox, bool forward)
{
	if (inputbox->parent == NULL) {
		return;
	}

	// Switch focus to the next/previous sibling.
	widget_t *child, *next = NULL;
	bool exit_on_next = false;

	if (forward) {

		list_foreach(inputbox->parent->children, child) {

			if (child->type != WIDGET_TYPE_INPUTBOX) {
				continue;
			}

			// Get the first possible inputbox as a backup if the current inputbox is the last one
			// in the list.
			if (next == NULL) {
				next = child;
			}

			// This is the next inputbox from the current one. Exit the search loop.
			if (exit_on_next) {

				next = child;
				break;
			}

			// Found the current inputbox. The next one is the one we're looking.
			if (child == inputbox) {
				exit_on_next = true;
			}
		}
	}
	else {

		list_foreach_reverse(inputbox->parent->children, child) {

			if (child->type != WIDGET_TYPE_INPUTBOX) {
				continue;
			}
			if (next == NULL) {
				next = child;
			}
			if (exit_on_next) {
				
				next = child;
				break;
			}
			if (child == inputbox) {
				exit_on_next = true;
			}
		}
	}

	// Switch focus.
	if (next != NULL &&
		next != inputbox) {

		mgui_set_focused_widget(next);

		// Select all text.
		next->inputbox.cursor_start = 0;
		next->inputbox.cursor_end = next->text->buffer_length;

		inputbox_update_cursor_position(next);
	}
}

static void inputbox_update_cursor_position(widget_t *inputbox)
{
	inputbox->inputbox.cursor_start_pos =
		text_calculate_width(inputbox->text, inputbox->inputbox.cursor_start);

	inputbox->inputbox.cursor_end_pos =
		text_calculate_width(inputbox->text, inputbox->inputbox.cursor_end);

	inputbox->has_colour_changed = true;
}

static void on_inputbox_focused(widget_t *inputbox, bool focused)
{
	UNUSED(focused);

	if (inputbox == NULL) {
		return;
	}

	// Refresh the vertices of the cursor to either dispay or hide it.
	inputbox->has_colour_changed = true;
}

static void on_inputbox_refresh_vertices(widget_t *inputbox)
{
	if (inputbox == NULL || inputbox->text == NULL || inputbox->inputbox.cursor == NULL) {
		return;
	}

	// Calculate cursor position.
	vec2_t position = inputbox->text->cursor_position;

	vec2_t min = vec2_zero();
	vec2_t max = vec2_zero();

	// Draw cursor when the widget is focused.
	if (inputbox->state & WIDGET_STATE_FOCUSED) {

		// Calculate the offset from text start to cursor position in pixels.
		float cursor_height = inputbox->text->font->height;
		float cursor_centre = inputbox->world_position.y + 0.5f * inputbox->size.y;

		// Clamp cursor/selection position to the boundaries of the inputbox.
		float cursor_start = inputbox->inputbox.cursor_start_pos;
		float cursor_end = inputbox->inputbox.cursor_end_pos;
		float width = 0;

		if (inputbox->inputbox.cursor_start == inputbox->inputbox.cursor_end) {
			width = inputbox->inputbox.cursor_width;
		}

		if (cursor_start < inputbox->text->position.x + inputbox->text->boundaries.x -
			               inputbox->text->margin.left - inputbox->text->margin.right) {

			float bounds_min_x = 0;
			float bounds_max_x = inputbox->text->position.x + inputbox->text->boundaries.x -
			                     inputbox->text->margin.left - inputbox->text->margin.right;

			cursor_start = CLAMP(cursor_start, bounds_min_x, bounds_max_x);
			cursor_end = CLAMP(cursor_end, bounds_min_x, bounds_max_x);

			min = vec2(position.x + cursor_start, cursor_centre - 0.5f * cursor_height);
			max = vec2(position.x + cursor_end + width, cursor_centre + 0.5f * cursor_height);
		}
	}

	// Get cursor sprite texture coordinates.
	sprite_t *sprite = inputbox->inputbox.cursor;
	vec2_t uv1 = sprite->uv1;
	vec2_t uv2 = sprite->uv2;

	float bottom = mgui_parameters.height;
	vertex_ui_t *vertices = inputbox->mesh->ui_vertices;

	vertices[16] = vertex_ui(vec2(min.x, bottom - min.y), vec2(uv1.x, uv1.y), COL_WHITE);
	vertices[17] = vertex_ui(vec2(min.x, bottom - max.y), vec2(uv1.x, uv2.y), COL_WHITE);
	vertices[18] = vertex_ui(vec2(max.x, bottom - min.y), vec2(uv2.x, uv1.y), COL_WHITE);
	vertices[19] = vertex_ui(vec2(max.x, bottom - max.y), vec2(uv2.x, uv2.y), COL_WHITE);
}

static void on_inputbox_pressed(widget_t *inputbox, bool pressed, int16_t x, int16_t y)
{
	UNUSED(y);

	if (inputbox == NULL) {
		return;
	}

	// Resolve the character index and position of the character under the cursor.
	int16_t text_x = x - widget_get_world_position(inputbox).x;
	int16_t char_pos_x;

	uint32_t cursor_position = text_get_closest_character(inputbox->text, text_x, &char_pos_x);

	// Update cursor/selection boundaries.
	if (pressed) {

		inputbox->inputbox.cursor_start = cursor_position;
		inputbox->inputbox.cursor_start_pos = char_pos_x;

		inputbox->inputbox.cursor_end = cursor_position;
		inputbox->inputbox.cursor_end_pos = char_pos_x;
	}
	else {

		inputbox->inputbox.cursor_end = cursor_position;
		inputbox->inputbox.cursor_end_pos = char_pos_x;
	}

	inputbox->has_colour_changed = true;
}

static void on_inputbox_dragged(widget_t *inputbox, int16_t x, int16_t y)
{
	// Dragging updates the selection the same way as if the cursor was released, so we'll just
	// let the press handler above do the work.
	on_inputbox_pressed(inputbox, false, x, y);
}

static bool on_inputbox_key_pressed(widget_t *inputbox, uint32_t key, bool pressed)
{
	if (inputbox == NULL || !pressed) {
		return true;
	}

	// TODO: Handle cut/copy/paste.

	switch (key) {

		case MKEY_ESCAPE:

			// Pressing escape removes focus from the widget.
			mgui_set_focused_widget(NULL);
			inputbox->has_colour_changed = true;

			return false;

		case MKEY_RETURN:

			// TODO: Add submitting.
			return false;

		case MKEY_LEFT:

			inputbox_move_cursor(inputbox, true);
			return false;

		case MKEY_RIGHT:

			inputbox_move_cursor(inputbox, false);
			return false;

		case MKEY_HOME:

			if (input_is_key_down(MKEY_SHIFT)) {

				// Select everything to the left.
				inputbox->inputbox.cursor_end = 0;
			}
			else {

				// Move cursor to the beginning of the line.
				inputbox->inputbox.cursor_start = 0;
				inputbox->inputbox.cursor_end = 0;
			}
			
			inputbox_update_cursor_position(inputbox);

			return false;

		case MKEY_END:

			if (input_is_key_down(MKEY_SHIFT)) {

				// Select everything to the right.
				inputbox->inputbox.cursor_end = inputbox->text->buffer_length;
			}
			else {

				// Move cursor to the beginning of the line.
				inputbox->inputbox.cursor_start = inputbox->text->buffer_length;
				inputbox->inputbox.cursor_end = inputbox->text->buffer_length;
			}

			inputbox_update_cursor_position(inputbox);

			return false;

		case MKEY_BACKSPACE:

			// Backspace removes a character to the left of cursor.
			inputbox_remove_character(inputbox, inputbox->inputbox.cursor_start);
			return false;

		case MKEY_DELETE:

			// Delete removes a character to the right of cursor.
			inputbox_remove_character(inputbox, inputbox->inputbox.cursor_start + 1);
			return false;

		case MKEY_TAB:

			// Change focus to either previous or next inputbox.
			inputbox_switch_focus(inputbox, !input_is_key_down(MKEY_SHIFT));
			return false;

		case 'A':

			if (input_is_key_down(MKEY_CONTROL)) {

				// Select all text.
				inputbox->inputbox.cursor_start = 0;
				inputbox->inputbox.cursor_end = inputbox->text->buffer_length;

				inputbox_update_cursor_position(inputbox);
			}
			return false;

		default:
			break;
	}

	return true;
}

static bool on_inputbox_character_injected(widget_t *inputbox, uint32_t c)
{
	if (inputbox == NULL) {
		return true;
	}

	if (inputbox->text->buffer_length >= MAX_INPUT_LENGTH) {
		return true;
	}

	// Inject visible characters into the text buffer.
	if (c < 32 || c > 255) {
		return true;
	}

	char tmp[MAX_INPUT_LENGTH + 1];
	size_t i = 0;

	uint32_t cursor_start;
	uint32_t cursor_end;

	if (inputbox->inputbox.cursor_start < inputbox->inputbox.cursor_end) {
		cursor_start = inputbox->inputbox.cursor_start;
		cursor_end = inputbox->inputbox.cursor_end;
	}
	else {
		cursor_start = inputbox->inputbox.cursor_end;
		cursor_end = inputbox->inputbox.cursor_start;
	}
	
	// If the buffer already has content and the cursor is somewhere else than the end of the
	// buffer, copy characters in a loop and inject the new character into the middle.
	if (inputbox->text->buffer != NULL &&
		inputbox->text->buffer_length != 0 &&
		(cursor_start != inputbox->text->buffer_length)) {

		char *s = inputbox->text->buffer;

		while (*s) {

			// Safety check so we don't write past the boundaries of the temporary buffer.
			if (i >= MAX_INPUT_LENGTH) {
				break;
			}

			if (i == cursor_start) {

				// Inject the character to the position of the cursor.
				tmp[i++] = (char)c;

				// Advance the cursor.
				inputbox->inputbox.cursor_start = i;
				inputbox->inputbox.cursor_end = i;

				// Skip the text that was selected.
				s += (cursor_end - cursor_start);
			}

			// Copy characters from the existing buffer.
			tmp[i++] = *s++;
		}
	}
	else {

		// Empty or non existing buffer or cursor at the end -> copy previous text into a new
		// buffer and append the injected character.
		i = inputbox->text->buffer_length;

		if (inputbox->text->buffer != NULL) {
			string_copy(tmp, inputbox->text->buffer, sizeof(tmp));
		}

		// Inject new character only if the buffer has space.			
		if (i < sizeof(tmp) - 1) {

			tmp[i++] = (char)c;
			inputbox->inputbox.cursor_start = i;
			inputbox->inputbox.cursor_end = i;
		}
	}

	// Terminate the buffer.
	tmp[i] = 0;

	// Copy the updated buffer into the text object.
	widget_set_text_s(inputbox, tmp);

	// Refresh cursor position.
	inputbox_update_cursor_position(inputbox);

	inputbox->has_colour_changed = true;
	return false;
}
