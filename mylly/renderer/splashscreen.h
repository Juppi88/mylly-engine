#pragma once
#ifndef __SPLASHSCREEN_H
#define __SPLASHSCREEN_H

#include "core/defines.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void splash_display(const char *logo_path, colour_t background_colour);
void splash_fade_out(void);

END_DECLARATIONS;

#endif
