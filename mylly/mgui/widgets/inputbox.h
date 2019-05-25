#pragma once
#ifndef __MGUI_INPUTBOX_H
#define __MGUI_INPUTBOX_H

#include "mgui/mgui.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

typedef struct inputbox_t {

	sprite_t *cursor;
	uint8_t cursor_width;

	uint32_t cursor_start;
	uint32_t cursor_end;
	int16_t cursor_start_pos;
	int16_t cursor_end_pos;

} inputbox_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *inputbox_create(widget_t *parent);

void inputbox_set_cursor_sprite(widget_t *inputbox, sprite_t *cursor, uint8_t width);
bool inputbox_has_selected_text(widget_t *inputbox);

END_DECLARATIONS;

#endif
