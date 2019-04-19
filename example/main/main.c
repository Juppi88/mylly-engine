#include "core/mylly.h"
#include "core/time.h"
#include "resources/resources.h"
#include "scene/model.h"
#include "scene/scene.h"
#include "scene/object.h"
#include "scene/camera.h"
#include "scene/light.h"
#include "math/math.h"
#include "io/input.h"

// --------------------------------------------------------------------------------

static scene_t *scene;
static object_t *camera;
static model_t *test_model;
static object_t *test, *test2;
static uint16_t mouse_x, mouse_y;
static object_t *spotlight;

// --------------------------------------------------------------------------------

typedef enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_EXIT,
	BUTTON_FORWARD,
	BUTTON_BACKWARD,
} key_bind_t;

KEYBIND_HANDLER(exit_app)
{
	if (!pressed) {
		return true;
	}

	printf("Exiting...\n");
	mylly_exit();

	return true;
}

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

	obj_set_local_position(camera, vector3(0.0f, 5.0f, 0.0f));
	obj_look_at(camera, vec3_zero(), vec3_right());
	//obj_set_local_rotation(camera, quat_from_euler(0, 0, DEG_TO_RAD(45)));

	camera_set_perspective_projection(camera->camera, 60, PERSPECTIVE_NEAR, PERSPECTIVE_FAR);
	//camera_set_orthographic_projection(camera->camera, 20, ORTOGRAPHIC_NEAR, ORTOGRAPHIC_FAR);

	// Create a test model (a quad) for testing.
	//test_model = model_create(NULL, NULL);
	//model_setup_primitive(test_model, PRIMITIVE_CUBE);
	//model_set_material(test_model, -1, res_get_shader("default-textured"), res_get_texture("pico"));
	//model_set_material(test_model, -1, res_get_shader("test-animated"), res_get_texture("animtest"));

	test_model = res_get_model("fighterjet");

	//model_set_material(test_model, -1, res_get_shader("default-textured"), res_get_texture("fighter"));

	// Create a test object and attach the model to it.
	test = scene_create_object(scene, NULL);
	obj_set_model(test, test_model);

	//sprite_t *sprite = res_get_sprite("pico");

	//obj_set_sprite(test, sprite);

	//obj_set_local_position(test, vector3(-0.25f, 0.5f, 0.0f));
	//obj_set_local_position(test, vector3(0.25f, 0.25f, 0));
	//obj_set_local_scale(test, vector3(0.5f, 0.5f, 1.0f));
	obj_set_local_rotation(test, quat_from_euler_deg(0, 0, 0));
	obj_set_local_scale(test, vec3(0.01f, 0.01f, 0.01f));

	// Create a light object and add it to the scene.
	object_t *light = scene_create_object(scene, NULL);
	obj_add_light(light);

	obj_set_position(light, vec3(0, 10, 5)); // 10*sin, 10*sin, 5

	light_set_type(light->light, LIGHT_POINT);
	light_set_colour(light->light, COL_BLUE);
	light_set_intensity(light->light, 3.0f);
	light_set_range(light->light, 50.0f);

	// Create a spotlight.
	spotlight = scene_create_object(scene, NULL);
	obj_add_light(spotlight);

	obj_set_position(spotlight, vec3(0, 2, 0)); // 10*sin, 10*sin, 5

	light_set_type(spotlight->light, LIGHT_SPOT);
	light_set_colour(spotlight->light, COL_WHITE);
	light_set_intensity(spotlight->light, 2.0f);
	light_set_range(spotlight->light, 20.0f);
	light_set_spotlight_cutoff_angle(spotlight->light, 10.0f, 15.0f);
	light_set_spotlight_direction(spotlight->light, vec3_subtract(vec3_zero(), obj_get_position(spotlight)));

	// Adjust ambient lighting.
	scene_set_ambient_light(scene, col(100, 100, 100));

/*
	float x = 10 * cos(2 * VectorArr[VEC_TIME].x);
	float y = 10 * sin(2 * VectorArr[VEC_TIME].x);

	// Red light
	lights[0].position = vec3(x, y, 5);
	lights[0].colour = vec3(0.1, 0.3, 1);
	lights[0].range = 15;
	lights[0].intensity = 3;

	// Static white light
	lights[1].position = vec3(0, 5, 10);
	//lights[1].colour = vec3(1, 1, 0.8);
	lights[1].colour = vec3(1, 1, 0.8);
	lights[1].range = 15;
	lights[1].intensity = 1.5;
*/

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
/*
	model_t *model = model_create("", "");
	model_setup_primitive(model, PRIMITIVE_CUBE);
	test2 = scene_create_object(scene, NULL);
	obj_set_model(test2, model);
*/
	// Get initial cursor position.
	input_get_cursor_position(&mouse_x, &mouse_y);

	// Bind virtual buttons.
	input_bind_button(BUTTON_UP, 'W');
	input_bind_button(BUTTON_LEFT, 'A');
	input_bind_button(BUTTON_DOWN, 'S');
	input_bind_button(BUTTON_RIGHT, 'D');
	input_bind_button(BUTTON_FORWARD, 'E');
	input_bind_button(BUTTON_BACKWARD, 'Q');

	// Exit the program when escape is pressed.
	input_bind_key(MKEY_ESCAPE, exit_app, NULL);

	mylly_set_scene(scene);

	// Add a post processing effect to the camera.
	camera_add_post_processing_effect(camera->camera, res_get_shader("effect-fxaa"));
	//camera_add_post_processing_effect(camera->camera, res_get_shader("effect-chromatic-aberration"));
	camera_add_post_processing_effect(camera->camera, res_get_shader("effect-glitch"));


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
	quat_t orientation = quat_multiply(direction, rotation);

	//obj_set_local_rotation(test, orientation);
	//obj_set_local_position(camera, vector3(0.5f, 0.5f, 0));
	//obj_set_local_rotation(camera, quat_from_euler(0, 0, angle));

	//mat_print(camera_get_view_matrix(camera->camera));

	mouse_x = x;
	mouse_y = y;

	// Handle camera movement.
	const float speed = 0.1f;

	static vec3_t position = vec3_zero();
	//vec3_t position = obj_get_position(camera);
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
	if (input_is_button_down(BUTTON_FORWARD)) {
		movement.z += 1.0f;
	}
	if (input_is_button_down(BUTTON_BACKWARD)) {
		movement.z -= 1.0f;
	}

	movement = vec3_multiply(movement, speed * get_time().delta_time);
	//position = vec3_add(&position, &movement);
	position = vec3_add(position, movement);

	// Update camera position.
	if (movement.x != 0 || movement.y != 0 || movement.z != 0) {

		obj_set_local_rotation(camera, quat_from_euler(position.x, position.y, position.z));
		printf("Camera rot: %f %f %f\n", position.x, position.y, position.z);
	}

	vec3_t world = camera_screen_to_world(camera->camera, vec3(x, y, 4));

	world.y = 5;
	obj_set_position(spotlight, world);


	//
	// END OF TEST CODE!
	//
}

int main(int argc, char **argv)
{
	// Initialize the engine and enter the main loop.
	if (mylly_initialize(argc, argv)) {

		setup();

		mylly_main_loop(main_loop);
	}

	//
	// TEST CODE!
	//

	scene_destroy(scene);
	//model_destroy(test_model);

	//
	// END OF TEST CODE!
	//

	return 0;
}
