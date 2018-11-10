#pragma once
#ifndef __CAMERA_H
#define __CAMERA_H

#include "core/defines.h"
#include "math/matrix.h"

BEGIN_DECLARATIONS;

typedef struct object_t object_t;

#define PERSPECTIVE_NEAR -0.1f
#define PERSPECTIVE_FAR -1000
#define ORTOGRAPHIC_NEAR 0.01f
#define ORTOGRAPHIC_FAR 10.0f

// --------------------------------------------------------------------------------

typedef struct camera_t {

	object_t *parent; // The object this camera is attached to
	uint32_t scene_index; // Index of this camera in the scene

	mat_t view; // The view matrix of this camera
	mat_t projection; // The projection matrix of this camera
	
	bool is_view_matrix_dirty; // True when the view matrix has not been updated
	bool is_projection_matrix_dirty; // True when the projection matrix has not been updated

	bool is_orthographic; // The camera is an orthographic one (as opposed to perspective)
	float near; // Near clip plane
	float far; // Far clip plane
	float size; // Size of the camera when ortographic
	float fov; // Field of view

} camera_t;

// --------------------------------------------------------------------------------

camera_t *camera_create(object_t *parent);
void camera_destroy(camera_t *camera);

// Set up an orthographic projection.
void camera_set_orthographic_projection(camera_t *camera, float size, float near, float far);

// Set up a perspective projection.
void camera_set_perspective_projection(camera_t *camera, float fov, float near, float far);

static INLINE const mat_t *camera_get_view_matrix(camera_t *camera);
static INLINE const mat_t *camera_get_projection_matrix(camera_t *camera);

void camera_update_view_matrix(camera_t *camera);
void camera_update_projection_matrix(camera_t *camera);

// --------------------------------------------------------------------------------

static INLINE const mat_t *camera_get_view_matrix(camera_t *camera)
{
	if (camera->is_view_matrix_dirty) {
		camera_update_view_matrix(camera);
	}

	return &camera->view;
}

static INLINE const mat_t *camera_get_projection_matrix(camera_t *camera)
{
	if (camera->is_projection_matrix_dirty) {
		camera_update_projection_matrix(camera);
	}

	return &camera->projection;
}

END_DECLARATIONS;

#endif
