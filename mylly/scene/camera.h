#pragma once
#ifndef __CAMERA_H
#define __CAMERA_H

#include "core/defines.h"
#include "collections/array.h"
#include "math/matrix.h"

// -------------------------------------------------------------------------------------------------

#define PERSPECTIVE_NEAR -0.1f
#define PERSPECTIVE_FAR -1000
#define ORTOGRAPHIC_NEAR 0.01f
#define ORTOGRAPHIC_FAR 10.0f

// -------------------------------------------------------------------------------------------------

typedef enum camera_state_t {

	CAMSTATE_VIEW_DIRTY = 0x1, // View matrix has not been updated since changes
	CAMSTATE_PROJ_DIRTY = 0x2, // Projection matrix has not been updated since changes
	CAMSTATE_VIEWPROJ_DIRTY = 0x4, // View-projection matrix has not been updated
	CAMSTATE_VIEWPROJ_INV_DIRTY = 0x8, // The inverse of view-projection matrix has not been updated

} camera_state_t;

// -------------------------------------------------------------------------------------------------

typedef struct camera_t {

	object_t *parent; // The object this camera is attached to
	uint32_t scene_index; // Index of this camera in the scene

	mat_t view; // The view matrix of this camera
	mat_t projection; // The projection matrix of this camera
	mat_t view_projection; // View and projection matrices combined
	mat_t view_projection_inv; // Inverse matrix of the above
	
	camera_state_t state; // Current state of camera matrices

	bool is_orthographic; // The camera is an orthographic one (as opposed to perspective)
	float clip_near; // Near clip plane
	float clip_far; // Far clip plane
	float size; // Size of the camera when ortographic
	float fov; // Field of view

	// List of post processing shaders to be applied to the camera's result image.
	arr_t(shader_t*) post_processing_effects;

} camera_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

camera_t *camera_create(object_t *parent);
void camera_destroy(camera_t *camera);

// Set up an orthographic projection.
void camera_set_orthographic_projection(camera_t *camera, float size, float near, float far);

// Set up a perspective projection.
void camera_set_perspective_projection(camera_t *camera, float fov, float near, float far);

// Convert coordinates from world space to screen space and vice versa. The Z coordinate in
// world-to-screen conversion return value is the depth value. When doing a screen-to-world
// conversion, the Z value in the screen coordinate is the distance from the camera towards
// the forward vector.
vec3_t camera_world_to_screen(camera_t *camera, vec3_t position);
vec3_t camera_screen_to_world(camera_t *camera, vec3_t position);

static INLINE const mat_t *camera_get_view_matrix(camera_t *camera);
static INLINE const mat_t *camera_get_projection_matrix(camera_t *camera);
static INLINE const mat_t *camera_get_view_projection_matrix(camera_t *camera);
static INLINE const mat_t *camera_get_view_projection_matrix_inverse(camera_t *camera);

void camera_update_view_matrix(camera_t *camera);
void camera_update_projection_matrix(camera_t *camera);
void camera_update_view_projection_matrix(camera_t *camera);
void camera_update_view_projection_matrix_inverse(camera_t *camera);

// Apply post-processing shaders to the camera's view.
void camera_add_post_processing_effect(camera_t *camera, shader_t *shader);

// -------------------------------------------------------------------------------------------------

static INLINE const mat_t *camera_get_view_matrix(camera_t *camera)
{
	if (camera->state & CAMSTATE_VIEW_DIRTY) {
		camera_update_view_matrix(camera);
	}

	return &camera->view;
}

static INLINE const mat_t *camera_get_projection_matrix(camera_t *camera)
{
	if (camera->state & CAMSTATE_PROJ_DIRTY) {
		camera_update_projection_matrix(camera);
	}

	return &camera->projection;
}

static INLINE const mat_t *camera_get_view_projection_matrix(camera_t *camera)
{
	if (camera->state & CAMSTATE_VIEWPROJ_DIRTY) {
		camera_update_view_projection_matrix(camera);
	}

	return &camera->view_projection;
}

static INLINE const mat_t *camera_get_view_projection_matrix_inverse(camera_t *camera)
{
	if (camera->state & CAMSTATE_VIEWPROJ_INV_DIRTY) {
		camera_update_view_projection_matrix_inverse(camera);
	}

	return &camera->view_projection_inv;
}

END_DECLARATIONS;

#endif
