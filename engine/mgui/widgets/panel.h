#pragma once
#ifndef __MGUI_PANEL_H
#define __MGUI_PANEL_H

#include "mgui/mgui.h"

// -------------------------------------------------------------------------------------------------

typedef struct panel_t {

	void *nothing;

} panel_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *panel_create(widget_t *parent);

END_DECLARATIONS;

#endif
