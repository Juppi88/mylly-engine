#include "mylly.h"
#include "io/log.h"
#include "platform/thread.h"
#include "platform/window.h"
#include "renderer/rendersystem.h"

static model_t *test_model;

bool mylly_initialize(int argc, char **argv)
{
	// Create the main window.
	if (!window_create(false, 640, 480)) {

		log_error("Mylly", "Unable to create main window.");
		return false;
	}

	// Initialize the render system.
	rsys_initialize();

	// Create a test model (a quad) for testing.
	test_model = model_create();
	model_setup_primitive(test_model, PRIMITIVE_QUAD);

	return true;
}

static void mylly_shutdown(void)
{
	model_destroy(test_model);
	
	rsys_shutdown();
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

		// Test rendering
		rsys_begin_frame();

		rsys_render_scene(test_model);

		rsys_end_frame();

		thread_sleep(10);
	}

	// Do cleanup when exiting the main loop.
	mylly_shutdown();
}
