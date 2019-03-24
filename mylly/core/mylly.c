#include "mylly.h"
#include "core/time.h"
#include "io/log.h"
#include "io/input.h"
#include "platform/thread.h"
#include "platform/window.h"
#include "platform/inputhook.h"
#include "renderer/rendersystem.h"
#include "renderer/debug.h"
#include "resources/resources.h"
#include "scene/scene.h"
#include "mgui/mgui.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

// -------------------------------------------------------------------------------------------------

static void mylly_set_working_directory(void);

// -------------------------------------------------------------------------------------------------

static scene_t *current_scene;
static bool is_running = true;
static monitor_info_t monitor; // Info about the monitor the engine is running on

// -------------------------------------------------------------------------------------------------

bool mylly_initialize(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	
	// Set working directory to the path of the executable.
	mylly_set_working_directory();

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

	time_initialize();

	// Initialize subsystems.
	rsys_initialize();
	input_initialize();

	// Load resources from files.
	res_initialize();

	// Initialize MGUI.
	mgui_parameters_t params = {
		monitor.width,
		monitor.height,
	};

	debug_initialize();

	mgui_initialize(params);

	return true;
}

static void mylly_shutdown(void)
{
	// Destroy UI.
	mgui_shutdown();

	debug_shutdown();
	
	// Unload all loaded resources.
	res_shutdown();
	
	input_shutdown();
	rsys_shutdown();
}

void mylly_main_loop(on_loop_t callback)
{
	// Enter the main loop.
	while (is_running) {

		// Process window events and input.
		window_pump_events();
		window_process_events(input_sys_process_messages);

		// Render the current scene.
		rsys_begin_frame();

		// Call the main loop callback.
		if (callback != NULL) {
			callback();
		}

		if (current_scene != NULL) {

			// Pre-process all objects in the current scene before rendering.
			scene_process_objects(current_scene);
			rsys_render_scene(current_scene);
		}

		// Update and render the UI.
		mgui_process();

		// Ending the frame will issue the actual draw calls.
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

static void mylly_set_working_directory(void)
{
	// TODO: Move this to platform specific code!

#ifndef _WIN32

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

#else

	// Get the full path for the executable.
	HMODULE executable = GetModuleHandleW(NULL);
	WCHAR path[MAX_PATH];

	DWORD read = GetModuleFileNameW(executable, path, MAX_PATH);

	// Null terminate the path and remove the name of the binary from it.
	path[read] = 0;

	for (DWORD i = read; i > 0; i--) {

		if (path[i] == '/') {
			path[i] = 0;
			break;
		}
	}

	// Set working directory to the path of the executable.
	SetCurrentDirectoryW(path);

#endif
}