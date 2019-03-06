#include "checkbox.h"
#include "mgui/text.h"
#include "mgui/widget.h"
#include "scene/sprite.h"
#include "renderer/mesh.h"
#include "renderer/font.h"
#include "io/log.h"
#include "core/string.h"

// -------------------------------------------------------------------------------------------------

static void inputbox_remove_character(widget_t *inputbox, uint32_t position);

static void on_inputbox_focused(widget_t *inputbox, bool focused);
static void on_inputbox_refresh_vertices(widget_t *inputbox);
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
	NULL,
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
		WIDGET_STATE_EXT_MESH
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

static void inputbox_remove_character(widget_t *inputbox, uint32_t position)
{
	// Nothing to remove if the cursor is at either end of the text.
	if (position == 0 || position > inputbox->text->buffer_length) {
		return;
	}

	char tmp[inputbox->text->buffer_length];
	char *s = inputbox->text->buffer;
	char *d = tmp;
	size_t i = 0;

	// Copy all the characters from the existing text buffer to the temporary buffer - except
	// for the one to be removed.
	while (*s) {

		if (i++ == position - 1) {

			++s;
			continue;
		}

		*d++ = *s++;
	}

	// Terminate the temporary buffer.
	*d = 0;

	// Copy the updated buffer into the text object.
	widget_set_text_s(inputbox, tmp);

	// Move cursor position back by one.
	inputbox->inputbox.cursor_position = position - 1;

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

	// TODO: Detect which part of the text was clicked. For now though, move the cursor to the end.
	inputbox->inputbox.cursor_position = inputbox->text->buffer_length;
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
		float cursor_offset = text_calculate_width(inputbox->text, inputbox->inputbox.cursor_position);
		float cursor_height = inputbox->text->font->height;
		float cursor_vert_offset = cursor_height - inputbox->text->font->size + 1;

		// Draw the cursor only when it's still within the text's boundaries. 
		if (position.x + cursor_offset <
			inputbox->text->position.x + inputbox->text->boundaries.x) {

			position.y += cursor_vert_offset;

			min = vec2(position.x + cursor_offset, position.y - cursor_height);
			max = vec2(min.x + inputbox->inputbox.cursor_width, position.y);
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

static bool on_inputbox_key_pressed(widget_t *inputbox, uint32_t key, bool pressed)
{
	if (inputbox == NULL || !pressed) {
		return true;
	}

	// TODO: Handle selecting text by holding shift.
	// TODO: Remove cut/copy/paste.

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

			// Move cursor to the left.
			if (inputbox->inputbox.cursor_position > 0) {

				inputbox->inputbox.cursor_position--;
				inputbox->has_colour_changed = true;
			}

			return false;

		case MKEY_RIGHT:

			// Move cursor to the right.
			if (inputbox->inputbox.cursor_position < inputbox->text->buffer_length) {

				inputbox->inputbox.cursor_position++;
				inputbox->has_colour_changed = true;
			}

			return false;

		case MKEY_HOME:

			// Move cursor to the beginning of the line.
			inputbox->inputbox.cursor_position = 0;
			inputbox->has_colour_changed = true;

			return false;

		case MKEY_END:

			// Move cursor to the beginning of the line.
			inputbox->inputbox.cursor_position = inputbox->text->buffer_length;
			inputbox->has_colour_changed = true;

			return false;

		case MKEY_BACKSPACE:

			// Backspace removes a character to the left of cursor.
			inputbox_remove_character(inputbox, inputbox->inputbox.cursor_position);
			return false;

		case MKEY_DELETE:

			// Delete removes a character to the right of cursor.
			inputbox_remove_character(inputbox, inputbox->inputbox.cursor_position + 1);
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

	// Inject visible characters into the text buffer.
	if (c < 32 || c > 255) {
		return true;
	}

	char tmp[1024];
	size_t i = 0;
	
	// If the buffer already has content and the cursor is somewhere else than the end of the
	// buffer, copy characters in a loop and inject the new character into the middle.
	if (inputbox->text->buffer != NULL &&
		inputbox->text->buffer_length != 0 &&
		(inputbox->inputbox.cursor_position != inputbox->text->buffer_length)) {

		char *s = inputbox->text->buffer;
	
		while (*s) {

			// Safety check so we don't write past the boundaries of the temporary buffer.
			if (i >= sizeof(tmp) - 1) {
				break;
			}

			if (i == inputbox->inputbox.cursor_position) {

				// Inject the character to the position of the cursor and advance the cursor.
				tmp[i++] = (char)c;
				inputbox->inputbox.cursor_position++;
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
			inputbox->inputbox.cursor_position++;
		}
	}

	// Terminate the buffer.
	tmp[++i] = 0;

	// Copy the updated buffer into the text object.
	widget_set_text_s(inputbox, tmp);

	inputbox->has_colour_changed = true;
	return false;
}
