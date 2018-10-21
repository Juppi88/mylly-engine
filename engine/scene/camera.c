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

	camera->is_orthographic = true;
	camera->size = size;
	camera->near = near;
	camera->far = far;

	camera->is_projection_matrix_dirty = true;
}

void camera_set_perspective_projection(camera_t *camera, float fov, float near, float far)
{
	if (camera == NULL) {
		return;
	}

	camera->is_orthographic = false;
	camera->fov = fov;
	camera->near = near;
	camera->far = far;

	camera->is_projection_matrix_dirty = true;
}

void camera_update_view_matrix(camera_t *camera)
{
	if (camera == NULL || !camera->is_view_matrix_dirty) {
		return;
	}

	object_t *obj = camera->parent;

	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	vec3_t right = vec3(
		obj->transform.col[0][0],
		obj->transform.col[1][0],
		obj->transform.col[2][0]
	);

	vec3_t up = vector3(
		obj->transform.col[0][1],
		obj->transform.col[1][1],
		obj->transform.col[2][1]
	);

	vec3_t forward = vector3(
		obj->transform.col[0][2],
		obj->transform.col[1][2],
		obj->transform.col[2][2]
	);

	mat_set(&camera->view,
			
		right.x,
		up.x,
		-forward.x,
		0,

		right.y,
		up.y,
		-forward.y,
		0,

		right.z,
		up.z,
		-forward.z,
		0,

		-vec3_dot(&obj->position, &obj->right),
		-vec3_dot(&obj->position, &obj->up),
		vec3_dot(&obj->position, &obj->forward),
		1
	);

	camera->is_view_matrix_dirty = false;
}

void camera_look_at(camera_t *camera, const vec3_t target, const vec3_t up)
{
	if (camera == NULL) {
		return;
	}

	vec3_t position = obj_get_position(camera->parent);

	vec3_t forward = vec3_subtract(&target, &position);
	vec3_normalize(&forward);

	vec3_t right = vec3_cross(&forward, &up);
	vec3_normalize(&right);

	vec3_t up2 = vec3_cross(&right, &forward);

	mat_set(&camera->view,
			
		right.x,
		up2.x,
		-forward.x,
		0,

		right.y,
		up2.y,
		-forward.y,
		0,

		right.z,
		up2.z,
		-forward.z,
		0,

		-vec3_dot(&position, &right),
		-vec3_dot(&position, &up2),
		vec3_dot(&position, &forward),
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
			-2.0f / (camera->far - camera->near),
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
			(camera->far + camera->near) / (camera->near - camera->far),
			-1,

			0,
			0,
			(2 * camera->far * camera->near) / (camera->near - camera->far),
			0
		);
	}

	camera->is_projection_matrix_dirty = false;
}
