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
	camera->near = -1;
	camera->far = 1;
	camera->size = 5;
	camera->fov = 60;

	//camera->near = 0.3f;
	//camera->far = 1000;

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

	camera->view.col[0][0] = right.x;
	camera->view.col[0][1] = right.y;
	camera->view.col[0][2] = right.z;

	camera->view.col[1][0] = up.x;
	camera->view.col[1][1] = up.y;
	camera->view.col[1][2] = up.z;

	camera->view.col[2][0] = -forward.x;
	camera->view.col[2][1] = -forward.y;
	camera->view.col[2][2] = -forward.z;

	camera->view.col[3][0] = -vec3_dot(&obj->position, &obj->right);
	camera->view.col[3][1] = -vec3_dot(&obj->position, &obj->up);
	camera->view.col[3][2] = -vec3_dot(&obj->position, &obj->forward);

	camera->view.col[0][3] = 0;
	camera->view.col[1][3] = 0;
	camera->view.col[2][3] = 0;
	camera->view.col[3][3] = 1;

	camera->is_view_matrix_dirty = false;
}

void camera_update_projection_matrix(camera_t *camera)
{
	if (camera == NULL || !camera->is_projection_matrix_dirty) {
		return;
	}

	// TODO: Get this from the rendering system!
	float aspect = 640.0f / 480.0f;

	if (camera->is_orthographic) {

		// Calculate an orthographic projection matrix.
		float width = aspect * camera->size;
		float left = -0.5f * width;
		float right = 0.5f * width;
		float top = 0.5f * camera->size;
		float bottom = -0.5f * camera->size;

		camera->projection.col[0][0] = 2.0f / (right - left);
		camera->projection.col[0][1] = 0;
		camera->projection.col[0][2] = 0;
		camera->projection.col[0][3] = 0;

		camera->projection.col[1][0] = 0;
		camera->projection.col[1][1] = 2.0f / (top - bottom);
		camera->projection.col[1][2] = 0;
		camera->projection.col[1][3] = 0;

		camera->projection.col[2][0] = 0;
		camera->projection.col[2][1] = -2.0f / (camera->far - camera->near);
		camera->projection.col[2][2] = 0;
		camera->projection.col[2][3] = 0;

		camera->projection.col[3][0] = -(right + left) / (right - left);
		camera->projection.col[3][1] = -(top + bottom) / (top - bottom);
		camera->projection.col[3][2] = -(camera->far + camera->near) / (camera->far - camera->near);
		camera->projection.col[3][3] = 1;
	}
	else {

		// Calculate a perspective projection matrix.
		float fov_y = DEG_TO_RAD(camera->fov);
		float f = 1 / tanf(0.5f * fov_y);

		//log_message(fov_y, )

		camera->projection.col[0][0] = f / aspect;
		camera->projection.col[0][1] = 0;
		camera->projection.col[0][2] = 0;
		camera->projection.col[0][3] = 0;

		camera->projection.col[1][0] = 0;
		camera->projection.col[1][1] = f;
		camera->projection.col[1][2] = 0;
		camera->projection.col[1][3] = 0;

		camera->projection.col[2][0] = 0;
		camera->projection.col[2][1] = (camera->far + camera->near) / (camera->near - camera->far);
		camera->projection.col[2][2] = 0;
		camera->projection.col[2][3] = -1;

		camera->projection.col[3][0] = 0;
		camera->projection.col[3][1] = 0;
		camera->projection.col[3][2] = (2 * camera->far * camera->near) /
									   (camera->near - camera->far);
		camera->projection.col[3][3] = 0;
	}

	camera->is_projection_matrix_dirty = false;
}
