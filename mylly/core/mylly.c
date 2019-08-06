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
#include "renderer/splashscreen.h"
#include "resources/resources.h"
#include "scene/scene.h"
#include "mgui/mgui.h"
#include "audio/audiosystem.h"
#include <stdio.h>
#include <string.h>

// -------------------------------------------------------------------------------------------------

static mylly_params_t parameters; // Engine initialization parameters
static scene_t *current_scene;
static bool is_running = true;
static monitor_info_t monitor; // Info about the monitor the engine is running on

// -------------------------------------------------------------------------------------------------

bool mylly_initialize(int argc, char **argv, const mylly_params_t *params)
{
	UNUSED(argc);
	UNUSED(argv);

#ifdef DEBUG
	// Ensure our debug messages are printed immediately so they don't get lost on segfault.
	setbuf(stdout, NULL);
#endif

	// Copy initialization parameters.
	if (params != NULL) {
		memcpy(&parameters, params, sizeof(parameters));
	}
	
	// Set working directory to the path of the executable.
	platform_set_working_directory();

	// Create the main window.
	// TODO: Figure out the coordinates.
	window_get_monitor_info(0, &monitor);

	if (!window_create(true, 0, 0, 0, monitor.width, monitor.height)) {

		log_error("Mylly", "Unable to create main window.");
		return false;
	}

	// Initialize the renderer.
	rsys_initialize();

	// Display the spash screen while other subsystems and recources are being loaded.
	splash_display(
		parameters.splash.logo_path,
		col(parameters.splash.r, parameters.splash.g, parameters.splash.b)
	);

	// Initialize other subsystems.
	parallel_initialize();
	input_initialize();
	audio_initialize();

	// Load resources from files.
	res_initialize();

	debug_initialize();

	// Initialize MGUI.
	mgui_parameters_t mgui_params = {
		monitor.width,
		monitor.height,
	};

	mgui_initialize(mgui_params);

	time_initialize();

	// Fade out the splash screen logo.
	splash_fade_out();

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

void mylly_main_loop(void)
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
		if (parameters.callbacks.on_loop != NULL) {
			parameters.callbacks.on_loop();
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
	if (parameters.callbacks.on_exit != NULL) {
		parameters.callbacks.on_exit();
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

const mylly_params_t *mylly_get_parameters(void)
{
	return &parameters;
}
