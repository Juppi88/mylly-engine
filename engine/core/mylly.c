#include "mylly.h"
#include "core/time.h"
#include "io/log.h"
#include "platform/thread.h"
#include "platform/window.h"
#include "renderer/rendersystem.h"
#include "framework/resources.h"
#include <unistd.h>

// --------------------------------------------------------------------------------

static void mylly_set_working_directory(void);

// --------------------------------------------------------------------------------

static scene_t *current_scene;

// --------------------------------------------------------------------------------

bool mylly_initialize(int argc, char **argv)
{
	// Set working directory to the path of the executable.
	mylly_set_working_directory();

	// Create the main window.
	if (!window_create(false, 640, 480)) {

		log_error("Mylly", "Unable to create main window.");
		return false;
	}

	time_initialize();

	// Initialize the render system.
	rsys_initialize();

	// Load resources from files.
	res_initialize();

	return true;
}

static void mylly_shutdown(void)
{
	// Unload all loaded resources.
	res_shutdown();
	
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

		// Render the current scene.
		rsys_begin_frame();

		if (current_scene != NULL) {
			rsys_render_scene(current_scene);
		}

		rsys_end_frame();

		time_tick();

		thread_sleep(10);
	}

	// Do cleanup when exiting the main loop.
	mylly_shutdown();
}

void mylly_set_scene(scene_t *scene)
{
	// TODO: Create a proper scene manager for scenes!
	current_scene = scene;
}

static void mylly_set_working_directory(void)
{
	// TODO: Move this to platform specific code!

	// Get the full path for the executable.
	char path[260];
	size_t read = readlink("/proc/self/exe", path, sizeof(path));

	// Null terminate the path and remove the name of the binary from it.
	path[read] = 0;

	for (size_t i = read; i > 0; i--) {
		if (path[i] == '/') {
			path[i] = 0;
			break;
		}
	}

	// Set working directory to the path of the executable.
	UNUSED_RETURN(chdir(path));
}
