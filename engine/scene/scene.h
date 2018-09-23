#pragma once
#ifndef __SCENE_H
#define __SCENE_H

#include "core/array.h"

typedef struct object_t object_t;
typedef struct camera_t camera_t;

// --------------------------------------------------------------------------------

typedef struct scene_t {

	arr_t(object_t*) objects; // List of all scene objects
	arr_t(object_t*) cameras; // List of all scene objects with a camera
} scene_t;

// --------------------------------------------------------------------------------

scene_t *scene_create(void);
void scene_destroy(scene_t *scene);

object_t *scene_create_object(scene_t *scene, object_t *parent);
void scene_register_camera(scene_t *scene, object_t *object);

void scene_remove_references_to_object(scene_t *scene, object_t *object);

#endif
