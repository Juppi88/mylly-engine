#include "mylly.h"
#include "io/log.h"
#include "platform/thread.h"
#include "platform/window.h"
#include "renderer/rendersystem.h"
#include "renderer/model.h"
#include "math/math.h"
#include "scene/scene.h"
#include "scene/object.h"
#include "scene/camera.h"

static scene_t *scene;
static object_t *camera;
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

	//
	// TEST CODE!
	//

	// Create a test scene.
	scene = scene_create();

	// Create a camera object and add it to the scene.
	camera = scene_create_object(scene, NULL);
	obj_add_camera(camera);

	obj_set_local_position(camera, vector3(-0.2f, 2.0f, 0.9f));
	//obj_set_local_rotation(camera, quat_from_euler(0, 0, DEG_TO_RAD(45)));

	// Create a test model (a quad) for testing.
	test_model = model_create();
	model_setup_primitive(test_model, PRIMITIVE_QUAD);

	// Create a test object and attach the model to it.
	test = scene_create_object(scene, NULL);
	test->model = test_model;

	obj_set_local_position(test, vector3(-0.25f, 0.5f, 0.0f));
	//obj_set_local_position(test, vector3(0.25f, 0.25f, 0));
	obj_set_local_scale(test, vector3(0.5f, 0.5f, 1.0f));
	obj_set_local_rotation(test, quat_from_euler(0, 0, DEG_TO_RAD(45)));

	// TEST CODE
	printf("Rot set: "); quat_print(quat_from_euler(0, 0, DEG_TO_RAD(45)));

	mat_print(obj_get_transform(test));

	printf("Pos: "); vec3_print(&test->position);
	printf("Scale: "); vec3_print(&test->scale);
	printf("Right: "); vec3_print(&test->right);
	printf("Up: "); vec3_print(&test->up);
	printf("Forward: "); vec3_print(&test->forward);
	printf("Rot local : "); quat_print(obj_get_local_rotation(test));
	printf("Rot global: "); quat_print(obj_get_rotation(test));
	// END OF TEST CODE

	mat_print(obj_get_transform(camera));
	mat_print(camera_get_view_matrix(camera->camera));

	// Create another object and attach it to the first one.
	test2 = scene_create_object(scene, test);
	test2->model = test_model;

	obj_set_local_position(test2, vector3(0.93f, 0.58f, 0));
	obj_set_local_scale(test2, vector3(0.75f, 0.5f, 0.5f));
	obj_set_local_rotation(test2, quat_from_euler(0, 0, DEG_TO_RAD(45)));

	//
	// END OF TEST CODE!
	//

	return true;
}

static void mylly_shutdown(void)
{
	//
	// TEST CODE!
	//

	scene_destroy(scene);
	model_destroy(test_model);

	//
	// END OF TEST CODE!
	//
	
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

		//
		// TEST CODE!
		//

		float angle = 0.005f * ++frames;
		//obj_set_local_rotation(test, quat_from_euler(angle, 0, 0));
		obj_set_local_position(camera, vector3(0.5f, 0.5f, 0));
		obj_set_local_rotation(camera, quat_from_euler(0, 0, angle));

		//mat_print(camera_get_view_matrix(camera->camera));

		rsys_render_scene(scene);

		//
		// END OF TEST CODE!
		//

		rsys_end_frame();

		thread_sleep(10);
	}

	// Do cleanup when exiting the main loop.
	mylly_shutdown();
}
