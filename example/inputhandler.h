#pragma once

#include "gamedefs.h"

// -------------------------------------------------------------------------------------------------

enum {
	BUTTON_FORWARD,
	BUTTON_BACKWARD,
	BUTTON_LEFT,
	BUTTON_RIGHT,

	NUM_CONTROLS
};

// -------------------------------------------------------------------------------------------------

class InputHandler
{
public:
	InputHandler(void);
	~InputHandler(void);

private:
	static bool ExitApp(uint32_t key, bool pressed, void *context);
};
