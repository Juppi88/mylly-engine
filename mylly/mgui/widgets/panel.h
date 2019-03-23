#pragma once
#ifndef __MGUI_PANEL_H
#define __MGUI_PANEL_H

#include "mgui/mgui.h"
#include "math/vector.h"
#include "io/input.h"

// -------------------------------------------------------------------------------------------------

typedef struct panel_t {

	bool use_custom_tex_coord;
	vec2_t uv1;
	vec2_t uv2;

} panel_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *panel_create(widget_t *parent);

void panel_set_custom_texture_coord(widget_t *panel, vec2_t uv1, vec2_t uv2);

END_DECLARATIONS;

#endif
