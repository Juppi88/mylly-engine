#pragma once
#ifndef __CAMERA_H
#define __CAMERA_H

#include "core/defines.h"
#include "math/matrix.h"

typedef struct object_t object_t;

// --------------------------------------------------------------------------------

typedef struct camera_t {

	object_t *parent; // The object this camera is attached to

	mat_t view; // The view matrix of this camera
	bool is_view_matrix_dirty; // True when the view matrix has not been updated

} camera_t;

// --------------------------------------------------------------------------------

camera_t *camera_create(object_t *parent);
void camera_destroy(camera_t *camera);

static INLINE const mat_t *camera_get_view_matrix(camera_t *camera);
void camera_update_view_matrix(camera_t *camera);

// --------------------------------------------------------------------------------

static INLINE const mat_t *camera_get_view_matrix(camera_t *camera)
{
	if (camera->is_view_matrix_dirty) {
		camera_update_view_matrix(camera);
	}

	return &camera->view;
}

#endif
