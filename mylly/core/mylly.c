#include "mylly.h"
#include "time.h"
#include "parallel.h"
#include "io/log.h"
#include "io/input.h"
#include "platform/thread.h"
#include "platform/window.h"
#include "platform/inputhook.h"
#include "platform/platform.h"
#include "renderer/rendersystem.h"
#include "renderer/debug.h"
#include "resources/resources.h"
#include "scene/scene.h"
#include "mgui/mgui.h"
#include "audio/audiosystem.h"
#include <stdio.h>

// -------------------------------------------------------------------------------------------------

static scene_t *current_scene;
static bool is_running = true;
static monitor_info_t monitor; // Info about the monitor the engine is running on

// -------------------------------------------------------------------------------------------------

bool mylly_initialize(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);

#ifdef DEBUG
	// Ensure our debug messages are printed immediately so they don't get lost on segfault.
	setbuf(stdout, NULL);
#endif
	
	// Set working directory to the path of the executable.
	platform_set_working_directory();

	// Create the main window.
	// TODO: Figure out the coordinates and the resolution.
	window_get_monitor_info(0, &monitor);

	/*monitor.width = 800;
	monitor.height = 600;

	if (!window_create(false, 0, 800, 450, 800, 600)) {*/
	if (!window_create(true, 0, 0, 0, monitor.width, monitor.height)) {

		log_error("Mylly", "Unable to create main window.");
		return false;
	}

	// Initialize subsystems.
	parallel_initialize();
	rsys_initialize();
	input_initialize();
	audio_initialize();

	// Load resources from files.
	res_initialize();

	// Initialize MGUI.
	mgui_parameters_t params = {
		monitor.width,
		monitor.height,
	};

	debug_initialize();

	mgui_initialize(params);

	time_initialize();

	return true;
}

static void mylly_shutdown(void)
{
	// Destroy UI.
	mgui_shutdown();

	debug_shutdown();

	// Unload all loaded resources.
	res_shutdown();

	audio_shutdown();
	input_shutdown();
	rsys_shutdown();
	parallel_shutdown();
}

void mylly_main_loop(on_loop_t loop_callback, on_exit_t exit_callback)
{
	// Enter the main loop.
	while (is_running) {

		// Process window events and input.
		window_pump_events();
		window_process_events(input_sys_process_messages);

		// Process parallel jobs.
		parallel_process();

		// Render the current scene.
		rsys_begin_frame();

		// Call the main loop callback.
		if (loop_callback != NULL) {
			loop_callback();
		}

		if (current_scene != NULL) {

			// Pre-process all objects in the current scene before rendering.
			scene_process_objects(current_scene);
			rsys_render_scene(current_scene);
		}

		// Update and render the UI.
		mgui_process();

		// Process audio listener.
		audio_update();

		// Ending the frame will issue the actual draw calls.
		rsys_end_frame(current_scene);

		time_tick();

		thread_sleep(10);
	}

	// Clean up game specific code.
	if (exit_callback != NULL) {
		exit_callback();
	}

	// Do cleanup when exiting the main loop.
	mylly_shutdown();
}

void mylly_set_scene(scene_t *scene)
{
	// TODO: Create a proper scene manager for scenes!
	current_scene = scene;
}

void mylly_exit(void)
{
	is_running = false;
}

void mylly_get_resolution(uint16_t *width, uint16_t *height)
{
	if (width != NULL && height != NULL) {

		*width = (uint16_t)monitor.width;
		*height = (uint16_t)monitor.height;
	}
}
