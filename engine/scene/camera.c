#include "camera.h"
#include "object.h"

camera_t *camera_create(object_t *parent)
{
	NEW(camera_t, camera);

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

	camera->view[0][0] = obj->forward.x;
	camera->view[1][0] = obj->forward.y;
	camera->view[2][0] = obj->forward.z;

	camera->view[0][1] = obj->right.x;
	camera->view[1][1] = obj->right.y;
	camera->view[2][1] = obj->right.z;

	camera->view[0][2] = obj->up.x;
	camera->view[1][2] = obj->up.y;
	camera->view[2][2] = obj->up.z;

	camera->view[3][0] = obj->position.x;
	camera->view[3][1] = obj->position.y;
	camera->view[3][2] = obj->position.z;

	camera->view[0][3] = 0;
	camera->view[1][3] = 0;
	camera->view[2][3] = 0;
	camera->view[3][3] = 1;

	camera->is_view_matrix_dirty = false;
}
