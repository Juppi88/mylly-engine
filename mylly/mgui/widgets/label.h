#pragma once
#ifndef __MGUI_LABEL_H
#define __MGUI_LABEL_H

#include "mgui/mgui.h"

// -------------------------------------------------------------------------------------------------

typedef struct label_t {

	bool resize_automatically;
	
} label_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

widget_t *label_create(widget_t *parent);
void label_set_resize_automatically(widget_t *label, bool resize);

END_DECLARATIONS;

#endif
