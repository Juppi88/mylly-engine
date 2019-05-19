#pragma once
#ifndef __SCENE_H
#define __SCENE_H

#include "collections/array.h"
#include "renderer/colour.h"

// -------------------------------------------------------------------------------------------------

typedef struct scene_t {

	arr_t(object_t*) objects; // List of all scene objects
	arr_t(object_t*) cameras; // List of all scene objects with a camera
	arr_t(object_t*) lights; // List of all scene objects with a light

	colour_t ambient_light; // Ambient light colour in this scene

} scene_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

scene_t *scene_create(void);
void scene_destroy(scene_t *scene);

void scene_process_objects(scene_t *scene);

object_t *scene_create_object(scene_t *scene, object_t *parent);
void scene_register_camera(scene_t *scene, object_t *object);
void scene_register_light(scene_t *scene, object_t *object);

void scene_remove_references_to_object(scene_t *scene, object_t *object);

camera_t *scene_get_main_camera(scene_t *scene);
void scene_set_ambient_light(scene_t *scene, colour_t light_colour);

END_DECLARATIONS;

#endif
