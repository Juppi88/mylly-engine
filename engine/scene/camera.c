#include "camera.h"
#include "object.h"
#include "io/log.h"
#include "math/math.h"

// --------------------------------------------------------------------------------

camera_t *camera_create(object_t *parent)
{
	NEW(camera_t, camera);

	camera->parent = parent;
	camera->scene_index = INVALID_INDEX;

	camera->is_view_matrix_dirty = true;
	camera->is_projection_matrix_dirty = true;

	camera->is_orthographic = true;
	camera->near = ORTOGRAPHIC_NEAR;
	camera->far = ORTOGRAPHIC_FAR;
	camera->size = 2;
	camera->fov = 60;

	return camera;
}

void camera_destroy(camera_t *camera)
{
	if (camera == NULL) {
		return;
	}

	DELETE(camera);
}

void camera_set_orthographic_projection(camera_t *camera, float size, float near, float far)
{
	if (camera == NULL) {
		return;
	}

	// Set camera parameters for orthographic projection.
	camera->is_orthographic = true;
	camera->size = size;
	camera->near = near;
	camera->far = far;

	// Flag the camera as dirty so the projection matrix is recalculated before use.
	camera->is_projection_matrix_dirty = true;
}

void camera_set_perspective_projection(camera_t *camera, float fov, float near, float far)
{
	if (camera == NULL) {
		return;
	}

	// Set camera parameters for perspective projection.
	camera->is_orthographic = false;
	camera->fov = fov;
	camera->near = near;
	camera->far = far;

	// Flag the camera as dirty so the projection matrix is recalculated before use.
	camera->is_projection_matrix_dirty = true;
}

void camera_update_view_matrix(camera_t *camera)
{
	if (camera == NULL || !camera->is_view_matrix_dirty) {
		return;
	}

	object_t *obj = camera->parent;

	// Update the camera object's transform matrix when it's not up to date.
	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	// Get up to date directional vectors.
	vec3_t right = obj_get_right_vector(obj);
	vec3_t up = obj_get_up_vector(obj);
	vec3_t forward = obj_get_forward_vector(obj);

	// Compose a left-hand view matrix from the direction vectors.
	mat_set(&camera->view,
			
		right.x,
		up.x,
		forward.x,
		0,

		right.y,
		up.y,
		forward.y,
		0,

		right.z,
		up.z,
		forward.z,
		0,

		-vec3_dot(right, obj->position),
		-vec3_dot(up, obj->position),
		-vec3_dot(forward, obj->position),
		1
	);

	camera->is_view_matrix_dirty = false;
}

void camera_update_projection_matrix(camera_t *camera)
{
	if (camera == NULL || !camera->is_projection_matrix_dirty) {
		return;
	}

	// TODO: Get this from the rendering system!
	float aspect = 800.0f / 600.0f;

	if (camera->is_orthographic) {

		// Calculate an orthographic projection matrix.
		float width = aspect * camera->size;
		float left = -0.5f * width;
		float right = 0.5f * width;
		float top = 0.5f * camera->size;
		float bottom = -0.5f * camera->size;

		mat_set(&camera->projection,

			2.0f / (right - left),
			0,
			0,
			0,

			0,
			2.0f / (top - bottom),
			0,
			0,

			0,
			0,
			2.0f / (camera->far - camera->near),
			0,

			-(right + left) / (right - left),
			-(top + bottom) / (top - bottom),
			-(camera->far + camera->near) / (camera->far - camera->near),
			1
		);

	}
	else {

		// Calculate a perspective projection matrix.
		float fov_y = DEG_TO_RAD(camera->fov);
		float f = 1 / tanf(0.5f * fov_y);

		mat_set(&camera->projection,

			f / aspect,
			0,
			0,
			0,

			0,
			f,
			0,
			0,

			0,
			0,
			(camera->far + camera->near) / (camera->far - camera->near),
			1,

			0,
			0,
			(2 * camera->far * camera->near) / (camera->far - camera->near),
			0
		);
	}

	camera->is_projection_matrix_dirty = false;
}
