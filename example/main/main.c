#include "core/mylly.h"
#include "core/time.h"
#include "resources/resources.h"
#include "renderer/model.h"
#include "scene/scene.h"
#include "scene/object.h"
#include "scene/camera.h"
#include "math/math.h"
#include "input/input.h"

// --------------------------------------------------------------------------------

static scene_t *scene;
static object_t *camera;
static model_t *test_model;
static object_t *test, *test2;
static uint16_t mouse_x, mouse_y;

// --------------------------------------------------------------------------------

typedef enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_EXIT,
} key_bind_t;

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
	//model_set_material(test_model, -1, res_get_shader("test-animated"), res_get_texture("animtest"));

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

	// Get initial cursor position.
	input_get_cursor_position(&mouse_x, &mouse_y);

	// Bind virtual buttons.
	input_bind_button(BUTTON_UP, 'W');
	input_bind_button(BUTTON_LEFT, 'A');
	input_bind_button(BUTTON_DOWN, 'S');
	input_bind_button(BUTTON_RIGHT, 'D');
	input_bind_button(BUTTON_EXIT, MKEY_ESCAPE);

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

	//float angle = get_time().time;
	//obj_set_local_rotation(test, quat_from_euler(DEG_TO_RAD(-45), angle, 0));

	uint16_t x, y;
	input_get_cursor_position(&x, &y);

	quat_t rotation = obj_get_local_rotation(test);
	quat_t direction = quat_from_euler(0, -DEG_TO_RAD(x - mouse_x), -DEG_TO_RAD(y - mouse_y));
	quat_t orientation = quat_multiply(&direction, &rotation);

	obj_set_local_rotation(test, orientation);
	//obj_set_local_position(camera, vector3(0.5f, 0.5f, 0));
	//obj_set_local_rotation(camera, quat_from_euler(0, 0, angle));

	//mat_print(camera_get_view_matrix(camera->camera));

	mouse_x = x;
	mouse_y = y;

	// Exit the program when escape is pressed.
	if (input_get_button_pressed(BUTTON_EXIT)) {

		printf("Exiting...\n");
		mylly_exit();
	}

	// Handle camera movement.
	const float speed = 1.0f;

	vec3_t position = obj_get_position(camera);
	vec3_t movement = vec3_zero();

	if (input_is_button_down(BUTTON_UP)) {
		movement.y += 1.0f;
	}
	if (input_is_button_down(BUTTON_DOWN)) {
		movement.y -= 1.0f;
	}
	if (input_is_button_down(BUTTON_LEFT)) {
		movement.x -= 1.0f;
	}
	if (input_is_button_down(BUTTON_RIGHT)) {
		movement.x += 1.0f;
	}

	movement = vec3_multiply(&movement, speed * get_time().delta_time);
	position = vec3_add(&position, &movement);

	// Update camera position.
	obj_set_position(camera, position);
	

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
