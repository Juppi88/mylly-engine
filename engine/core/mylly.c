#include "mylly.h"
#include "io/log.h"
#include "platform/thread.h"
#include "platform/window.h"
#include "renderer/rendersystem.h"
#include "math/math.h"

static model_t *test_model;
static object_t *test, *test2;

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

	// Create a test object and attach the model to it.
	test = obj_create(NULL);
	test->model = test_model;

	// Create another object and attach it to the first one.
	test2 = obj_create(test);
	test2->model = test_model;

	obj_set_local_position(test2, vector3(0.93f, 0.58f, 0));
	obj_set_local_scale(test2, vector3(0.75f, 0.5f, 0.5f));
	obj_set_local_rotation(test2, quat_from_euler(0, 0, DEG_TO_RAD(45)));

	return true;
}

static void mylly_shutdown(void)
{
	obj_destroy(test);
	obj_destroy(test2);
	model_destroy(test_model);
	
	rsys_shutdown();
}

void mylly_main_loop(on_loop_t callback)
{
	static int frames = 0;

	// Enter the main loop.
	for (;;) {

		// Call the main loop callback.
		if (callback != NULL) {
			callback();
		}

		window_pump_events();

		// Test rendering
		rsys_begin_frame();

		float angle = 0.01f * ++frames;

		obj_set_local_position(test, vector3(0.25f, 0.25f, 0));
		obj_set_local_scale(test, vector3(0.5f, 0.5f, 0.5f));
		obj_set_local_rotation(test, quat_from_euler(0, 0, angle));

		rsys_render_scene(test);

		rsys_end_frame();

		thread_sleep(10);
	}

	// Do cleanup when exiting the main loop.
	mylly_shutdown();
}
