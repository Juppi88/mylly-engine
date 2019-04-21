#include "inputhandler.h"
#include <mylly/core/mylly.h>
#include <mylly/io/input.h>
#include <mylly/io/log.h>

// -------------------------------------------------------------------------------------------------

InputHandler::InputHandler(void)
{
	// Bind virtual buttons.
	input_bind_button(BUTTON_FORWARD, 'W');
	input_bind_button(BUTTON_BACKWARD, 'S');
	input_bind_button(BUTTON_LEFT, 'A');
	input_bind_button(BUTTON_RIGHT, 'D');

	// Exit the program when escape is pressed.
	input_bind_key(MKEY_ESCAPE, ExitApp, nullptr);
}

InputHandler::~InputHandler(void)
{
	
}

bool InputHandler::ExitApp(uint32_t key, bool pressed, void *context)
{
	if (!pressed) {
		return true;
	}

	log_message("Game", "Exiting...");
	mylly_exit();

	return true;
}
