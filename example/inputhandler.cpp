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
	input_bind_button(BUTTON_FIRE, MKEY_SPACE);
	input_bind_button(BUTTON_CONFIRM, MKEY_RETURN);
	input_bind_button(BUTTON_GODMODE, 'G');

	// Exit the program when escape is pressed.
	input_bind_key(MKEY_ESCAPE, ExitApp, nullptr);
}

InputHandler::~InputHandler(void)
{
	
}

float InputHandler::GetSteering(void) const
{
	float direction = 0;
	
	if (input_is_button_down(BUTTON_LEFT)) {
		direction += -1;
	}
	if (input_is_button_down(BUTTON_RIGHT)) {
		direction += 1;
	}

	return direction;
}

float InputHandler::GetAcceleration(void) const
{
	return (input_is_button_down(BUTTON_FORWARD) ? 1 : 0);
}

bool InputHandler::IsFiring(void) const
{
	return input_is_button_down(BUTTON_FIRE);
}

bool InputHandler::IsPressingConfirm(void) const
{
	return input_is_button_down(BUTTON_CONFIRM);
}

bool InputHandler::IsPressingGodmodeButton(void) const
{
	return input_is_button_down(BUTTON_GODMODE);
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
