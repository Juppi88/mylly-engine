#include "core/mylly.h"
#include "framework/resources.h"
#include "renderer/model.h"
#include "scene/scene.h"
#include "scene/object.h"
#include "scene/camera.h"
#include "math/math.h"

// --------------------------------------------------------------------------------

static int frames = 0;
static scene_t *scene;
static object_t *camera;
static model_t *test_model;
static object_t *test, *test2;

// --------------------------------------------------------------------------------

static void setup(void)
{
	//
	// TEST CODE!
	//

	// Create a test scene.
	scene = scene_create();

	// Create a camera object and add it to the scene.
	camera = scene_create_object(scene, NULL);
	obj_add_camera(camera);

	obj_set_local_position(camera, vector3(0.0f, 0.0f, 5.0f));
	//obj_set_local_rotation(camera, quat_from_euler(0, 0, DEG_TO_RAD(45)));

	camera_set_perspective_projection(camera->camera, 20, PERSPECTIVE_NEAR, PERSPECTIVE_FAR);

	// Create a test model (a quad) for testing.
	test_model = model_create();
	model_setup_primitive(test_model, PRIMITIVE_CUBE);
	model_set_material(test_model, -1, res_get_shader("default-textured"), res_get_texture("pico"));

	// Create a test object and attach the model to it.
	test = scene_create_object(scene, NULL);
	test->model = test_model;

	//obj_set_local_position(test, vector3(-0.25f, 0.5f, 0.0f));
	//obj_set_local_position(test, vector3(0.25f, 0.25f, 0));
	//obj_set_local_scale(test, vector3(0.5f, 0.5f, 1.0f));
	//obj_set_local_rotation(test, quat_from_euler(0, DEG_TO_RAD(45), 0));

	// TEST CODE
	/*printf("Rot set: "); quat_print(quat_from_euler(0, 0, DEG_TO_RAD(45)));

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
	mat_print(camera_get_view_matrix(camera->camera));*/

	// Create another object and attach it to the first one.
	/*test2 = scene_create_object(scene, test);
	test2->model = test_model;

	obj_set_local_position(test2, vector3(0.93f, 0.58f, 0));
	obj_set_local_scale(test2, vector3(0.75f, 0.5f, 0.5f));
	obj_set_local_rotation(test2, quat_from_euler(0, 0, DEG_TO_RAD(45)));
	*/

	mylly_set_scene(scene);


	//
	// END OF TEST CODE!
	//
}

static void main_loop(void)
{
	//
	// TEST CODE!
	//

	float angle = 0.005f * ++frames;
	obj_set_local_rotation(test, quat_from_euler(DEG_TO_RAD(-45), angle, 0));
	//obj_set_local_position(camera, vector3(0.5f, 0.5f, 0));
	//obj_set_local_rotation(camera, quat_from_euler(0, 0, angle));

	//mat_print(camera_get_view_matrix(camera->camera));
	

	//
	// END OF TEST CODE!
	//
}

int main(int argc, char **argv)
{
	// Initiliaze the engine and enter the main loop.
	if (mylly_initialize(argc, argv)) {

		setup();

		mylly_main_loop(main_loop);
	}

	//
	// TEST CODE!
	//

	scene_destroy(scene);
	model_destroy(test_model);

	//
	// END OF TEST CODE!
	//

	return 0;
}
