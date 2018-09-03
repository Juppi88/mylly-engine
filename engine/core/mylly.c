#include "mylly.h"
#include "io/log.h"
#include "platform/thread.h"
#include "platform/window.h"

bool mylly_initialize(int argc, char **argv)
{
	// Create the main window.
	if (!window_create(false, 640, 480)) {

		log_error("Mylly", "Unable to create main window.");
		return false;
	}

	return true;
}

void mylly_main_loop(on_loop_t callback)
{
	// Enter the main loop.
	for (;;) {

		// Call the main loop callback.
		if (callback != NULL) {
			callback();
		}

		window_pump_events();
		thread_sleep(10);
	}
}
