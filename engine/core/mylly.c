#include "mylly.h"
#include "io/log.h"
#include "platform/thread.h"
#include "platform/window.h"
#include "renderer/renderer.h"

bool mylly_initialize(int argc, char **argv)
{
	// Create the main window.
	if (!window_create(false, 640, 480)) {

		log_error("Mylly", "Unable to create main window.");
		return false;
	}

	// Initialize the renderer subsystem.
	rend_initialize();

	return true;
}

static void mylly_shutdown(void)
{
	rend_shutdown();
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

		rend_draw_view();

		thread_sleep(10);
	}

	// Do cleanup when exiting the main loop.
	mylly_shutdown();
}
