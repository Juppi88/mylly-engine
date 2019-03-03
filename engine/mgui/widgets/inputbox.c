#include "checkbox.h"
#include "mgui/text.h"
#include "mgui/widget.h"
#include "scene/sprite.h"
#include "renderer/mesh.h"
#include "io/log.h"
#include "core/string.h"

// -------------------------------------------------------------------------------------------------

static void on_inputbox_focused(widget_t *inputbox, bool focused);
static void on_inputbox_refresh_vertices(widget_t *inputbox);
static void on_inputbox_key_pressed(widget_t *inputbox, uint32_t key);

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

	return widget;
}

void inputbox_set_cursor_sprite(widget_t *inputbox, sprite_t *cursor, uint8_t margin)
{
	if (inputbox == NULL || inputbox->type != WIDGET_TYPE_INPUTBOX || cursor == NULL) {
		return;
	}

	inputbox->inputbox.cursor = cursor;
	inputbox->inputbox.cursor_margin = margin;

	// Refresh the vertices for the cursor.
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
	if (inputbox == NULL || inputbox->inputbox.cursor == NULL) {
		return;
	}

	// Calculate cursor position.
	float margin = (float)inputbox->inputbox.cursor_margin;
	float bottom = mgui_parameters.height;

	vec2_t min = vec2_zero();
	vec2_t max = vec2_zero();

	if (inputbox->state & WIDGET_STATE_FOCUSED) {

		min = vec2(inputbox->world_position.x + margin, inputbox->world_position.y + margin);
		max = vec2(min.x + 2, min.y + inputbox->size.y - 2 * margin);
	}

	// Get cursor sprite texture coordinates.
	sprite_t *sprite = inputbox->inputbox.cursor;
	vec2_t uv1 = sprite->uv1;
	vec2_t uv2 = sprite->uv2;

	vertex_ui_t *vertices = inputbox->mesh->ui_vertices;

	vertices[16] = vertex_ui(vec2(min.x, bottom - min.y), vec2(uv1.x, uv1.y), COL_WHITE);
	vertices[17] = vertex_ui(vec2(min.x, bottom - max.y), vec2(uv1.x, uv2.y), COL_WHITE);
	vertices[18] = vertex_ui(vec2(max.x, bottom - min.y), vec2(uv2.x, uv1.y), COL_WHITE);
	vertices[19] = vertex_ui(vec2(max.x, bottom - max.y), vec2(uv2.x, uv2.y), COL_WHITE);
}

static void on_inputbox_key_pressed(widget_t *inputbox, uint32_t key)
{
	if (inputbox == NULL) {
		return;
	}

	// TODO: Handle arrow keys, backspace, delete, return etc.
	// TODO: Handle num keys
	// TODO: Handle lower case keys (use character event instead of key down?)
	// TODO: Add margin for text object in widget?

	// Pressing escape removes focus from the widget.
	if (key == MKEY_ESCAPE) {
		mgui_set_focused_widget(NULL);
	}

	// Inject visible characters into the text buffer.
	if (key >= 32 && key <= 255) {

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
					tmp[i++] = (char)key;
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

				tmp[i++] = (char)key;
				inputbox->inputbox.cursor_position++;
			}
		}

		// Terminate the buffer.
		tmp[++i] = 0;

		// Copy the updated buffer into the text object.
		widget_set_text_s(inputbox, tmp);
	}
}
