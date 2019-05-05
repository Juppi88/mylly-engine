#include "scene.h"
#include "object.h"
#include "camera.h"
#include "light.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

scene_t *scene_create(void)
{
	NEW(scene_t, scene);

	arr_init(scene->objects);
	arr_init(scene->cameras);
	arr_init(scene->lights);

	scene->ambient_light = col(25, 25, 25);

	return scene;
}

void scene_destroy(scene_t *scene)
{
	if (scene == NULL) {
		return;
	}

	object_t *obj;

	// Remove references to this scene in the objects and destroy them.
	arr_foreach_reverse(scene->objects, obj) {

		// Remove the scene reference to speed up the destroying
		// (skips unnecessary additional cleanup).
		if (obj != NULL) {

			obj->scene = NULL;
			obj_destroy(obj);
		}
	}

	// Destroy the scene.
	DESTROY(scene);
}

void scene_process_objects(scene_t *scene)
{
	if (scene == NULL) {
		return;
	}

	// Process all objects added to the scene.
	object_t *obj;
	arr_foreach(scene->objects, obj) {

		if (obj != NULL && obj->is_active) {
			obj_process(obj);
		}
	}
}

object_t *scene_create_object(scene_t *scene, object_t *parent)
{
	if (scene == NULL) {
		return NULL;
	}

	// Create the object.
	object_t *object = obj_create(scene, parent);

	if (object == NULL) {
		return NULL;
	}

	// Find a free object index.
	uint32_t index;
	arr_find_empty(scene->objects, index);

	// If there are no free indices in the scene, create new ones by pushing empty objects.
	if (index == INVALID_INDEX) {

		arr_push(scene->objects, NULL);
		index = arr_last_index(scene->objects);
	}

	// Add the object to the scene.
	arr_set(scene->objects, index, object);
	object->scene_index = index;

	return object;
}

void scene_register_camera(scene_t *scene, object_t *object)
{
	if (scene == NULL || object == NULL || object->camera == NULL) {
		return;	
	}

	// Ensure the object is in this scene.
	if (object->scene != scene) {
		log_warning("Scene", "Failed to register a camera: the object is in a different scene.");
		return;
	}

	// Make sure the camera is not added to a scene already.
	if (object->camera->scene_index != INVALID_INDEX) {
		log_warning("Scene", "Failed to register a camera: the camera is already in a scene.");
		return;	
	}

	// Find a free camera index.
	uint32_t index;
	arr_find_empty(scene->cameras, index);

	// If there are no free indices in the scene, create new ones by pushing empty objects.
	if (index == INVALID_INDEX) {

		arr_push(scene->cameras, NULL);
		index = arr_last_index(scene->cameras);
	}

	// Add the camera to the scene.
	arr_set(scene->cameras, index, object);
	object->camera->scene_index = index;
}

void scene_register_light(scene_t *scene, object_t *object)
{
	if (scene == NULL || object == NULL || object->light == NULL) {
		return;
	}

	// Ensure the object is in this scene.
	if (object->scene != scene) {

		log_warning("Scene", "Failed to register a light: the object is in a different scene.");
		return;
	}

	// Make sure the light is not added to the scene already.
	int index;
	arr_find(scene->lights, object, index);

	if (index >= 0) {

		log_warning("Scene", "Failed to register a light: the light is already in a scene.");
		return;
	}

	// Find a free light index.
	uint32_t free_index;
	arr_find_empty(scene->lights, free_index);

	// If there are no free indices in the scene, create new ones by pushing empty objects.
	if (free_index == INVALID_INDEX) {

		arr_push(scene->lights, NULL);
		free_index = arr_last_index(scene->lights);
	}

	// Add the light to the scene.
	arr_set(scene->lights, free_index, object);
	object->light->scene_index = free_index;
}

void scene_remove_references_to_object(scene_t *scene, object_t *object)
{
	if (scene == NULL || object == NULL) {
		return;
	}

	// Make sure the object is actually in this scene.
	if (scene != object->scene) {

		log_warning("Scene", "Failed to remove references: the object is in a different scene.");
		return;
	}

	// Remove references to special components if they exist.
	if (object->camera != NULL &&
		object->camera->scene_index != INVALID_INDEX) {

		arr_set(scene->cameras, object->camera->scene_index, NULL);
	}

	if (object->light != NULL &&
		object->light->scene_index != INVALID_INDEX) {

		arr_set(scene->lights, object->light->scene_index, NULL);
	}

	// Remove reference to the object itself.
	arr_set(scene->objects, object->scene_index, NULL);
}

void scene_set_ambient_light(scene_t *scene, colour_t light_colour)
{
	if (scene == NULL) {
		return;
	}

	scene->ambient_light = light_colour;
}
