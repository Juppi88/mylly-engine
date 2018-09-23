#include "camera.h"
#include "object.h"

camera_t *camera_create(object_t *parent)
{
	NEW(camera_t, camera);

	camera->scene_index = INVALID_INDEX;
	camera->parent = parent;
	camera->is_view_matrix_dirty = true;

	return camera;
}

void camera_destroy(camera_t *camera)
{
	if (camera == NULL) {
		return;
	}

	DELETE(camera);
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
