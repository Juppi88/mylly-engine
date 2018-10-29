#include "object.h"
#include "camera.h"
#include "scene.h"
#include "io/log.h"
#include "math/math.h"
#include "renderer/model.h"

// --------------------------------------------------------------------------------

object_t *obj_create(scene_t *scene, object_t *parent)
{
	// Make sure the object is created to a scene to avoid leaking objects.
	if (scene == NULL) {
		log_error("Scene", "Can't create an object without a scene.");
		return NULL;
	}

	// Create the object.
	NEW(object_t, obj);

	obj->parent = NULL;
	obj->scene = scene;
	obj->scene_index = INVALID_INDEX;

	// Move the object to the world origin.
	obj->local_position = vec3_zero;
	obj->local_scale = vec3_one;
	obj->local_rotation = quat_identity;

	obj->is_transform_dirty = true;
	obj->is_local_transform_dirty = true;
	obj->is_rotation_dirty = true;

	// Attach the object to a parent.
	if (parent != NULL) {
		obj_set_parent(obj, parent);
	}

	return obj;
}

void obj_destroy(object_t *obj)
{
	if (obj == NULL) {
		return;
	}

	// Detach from parent.
	if (obj->parent != NULL) {
		obj_set_parent(obj, NULL);
	}

	// Additional cleanup when the object is still in a scene. If the object is no longer in a
	// scene, the parent scene is being deleted and no additional clean up is necessary.
	if (obj->scene != NULL) {

		// Remove object references in the scene.
		scene_remove_references_to_object(obj->scene, obj);

		// Destroy all child objects.
		object_t *child;

		arr_foreach_reverse(obj->children, child) {
			obj_destroy(child);
		}
	}

	// Destroy components.
	if (obj->camera != NULL) {
		camera_destroy(obj->camera);
	}

	DELETE(obj);
}

void obj_set_parent(object_t *obj, object_t *parent)
{
	if (obj == NULL) {
		return;
	}

	// Remove this object from the old parent.
	if (obj->parent != NULL) {
		arr_remove(obj->parent->children, obj);
	}

	// Add this object to the new parent.
	if (parent != NULL) {

		if (parent->scene == obj->scene) {

			arr_push(parent->children, obj);
			obj->parent = parent;
		}
		else {
			log_error("Scene", "Parent object is in a different scene.");
		}
	}
	else {
		obj->parent = NULL;
	}

	// Flag the model matrix dirty.
	obj_set_dirty(obj);
}

camera_t *obj_add_camera(object_t *obj)
{
	// One camera per object.
	if (obj == NULL || obj->camera != NULL) {
		return NULL;
	}

	obj->camera = camera_create(obj);

	// Register the new camera to the object's parent scene.
	if (obj->scene != NULL) {
		scene_register_camera(obj->scene, obj);
	}

	return obj->camera;
}

void obj_look_at(object_t *obj, const vec3_t target, const vec3_t upward)
{
	if (obj == NULL) {
		return;
	}

	// Calculate forward, right and up vectors.
	vec3_t position = obj_get_position(obj);

	vec3_t forward = vec3_subtract(target, position);
	vec3_normalize(forward);


	//forward = vec3_multiply(&forward, -1);

	vec3_t right = vec3_cross(forward, upward);
	vec3_normalize(right);

	vec3_t up = vec3_cross(right, forward);

	printf("Pos: "); vec3_print(position);
	printf("Tgt: "); vec3_print(target);
	printf("Upw: "); vec3_print(upward);
	printf("\n");
	printf("Fwd: "); vec3_print(forward);
	printf("Up : "); vec3_print(up);
	printf("Rgt: "); vec3_print(right);

	// Calculate the quaternion from the direction vectors.
	quat_t quat;


	float x, y, z;
	y = asinf(-forward.x);

	float cy = sqrtf(1 - forward.z * forward.z);

	if (cy == 0 || fabsf(forward.z) >= 1) {

		// Looking straight up or down.
		if (y > 0) {
			z = 0;
			x = atan2f(-up.y, -up.x) + PI;
		}
		else {
			z = 0;
			x = -atan2f(up.y, up.x) + PI;
		}
	}
	else {

		float cz = forward.x / cy;
		float sz = forward.y / cy;

		z = atan2f(sz, cz);

		float cx = up.z / cy;
		float sx;

		if (fabsf(cz) < fabsf(sz)) {
			sx = -(up.x + forward.z * cx * cz) / sz;
		}
		else {
			sx = (up.y + forward.z * cx * sz) / cz;
		}

		x = atan2f(sx, cx);
	}

	vec3_t euler = vec3(x, y, z);

	//
	quat = quat_from_euler3(euler);
	

	obj->local_rotation = quat;
	obj_set_dirty(obj);


	printf("----------\n");
	forward = obj_get_forward_vector(obj);
	up = obj_get_up_vector(obj);
	right = obj_get_right_vector(obj);
	printf("Fwd: "); vec3_print(forward);
	printf("Up : "); vec3_print(up);
	printf("Rgt: "); vec3_print(right);

	printf("Transform:\n");
	mat_print(&obj->transform);

	/*vec3_t euler = vec3(DEG_TO_RAD(30), DEG_TO_RAD(60), DEG_TO_RAD(70));
	quat = quat_from_euler3(&euler);
	printf("Euler: %.1f %.1f %.1f\n", RAD_TO_DEG(euler.x), RAD_TO_DEG(euler.y), RAD_TO_DEG(euler.z));

	euler = quat_to_euler(&quat);
	*/
	vec3_t euler2 = quat_to_euler(quat);
	printf("Quat: "); quat_print(quat);
	printf("Euler1: %.1f %.1f %.1f\n", RAD_TO_DEG(euler.x), RAD_TO_DEG(euler.y), RAD_TO_DEG(euler.z));
	printf("Euler2: %.1f %.1f %.1f\n", RAD_TO_DEG(euler2.x), RAD_TO_DEG(euler2.y), RAD_TO_DEG(euler2.z));

	vec3_t result_rgt = vec3(1, 0, 0);
	vec3_t result_upw = vec3(0, 1, 0);
	vec3_t result_fwd = vec3(0, 0, 1);

	result_rgt = quat_rotate_vec3(quat, result_rgt);
	result_upw = quat_rotate_vec3(quat, result_upw);
	result_fwd = quat_rotate_vec3(quat, result_fwd);

	printf("----------\n");

	printf("MULT Fwd: "); vec3_print(result_fwd);
	printf("MULT Up : "); vec3_print(result_upw);
	printf("MULT Rgt: "); vec3_print(result_rgt);
}

void obj_set_dirty(object_t *obj)
{
	if (obj == NULL) {
		return;
	}

	// Flag the object itself as dirty.
	obj->is_transform_dirty = true;
	obj->is_local_transform_dirty = true;
	obj->is_rotation_dirty = true;

	// Flag components dirty.
	if (obj->camera != NULL) {
		obj->camera->is_view_matrix_dirty = true;
	}

	// Do the same to each child object.
	object_t *child;

	arr_foreach(obj->children, child) {
		obj_set_dirty(child);
	}
}

void obj_update_transform(object_t *obj)
{
	if (obj == NULL || !obj->is_transform_dirty) {
		return;
	}

	if (obj->parent != NULL) {

		// Calculate object transform by multiplying the parent's transform and the local transform.
		mat_multiply(
			*obj_get_transform(obj->parent),
			*obj_get_local_transform(obj),
			&obj->transform
		);
	}
	else {

		// If the object has no parent, its transform is just the local transform.
		mat_cpy(&obj->transform, obj_get_local_transform(obj));
	}

	// Cache world space position, scale and direction vectors.
	obj->position = vector3(
			obj->transform.col[3][0],
			obj->transform.col[3][1],
			obj->transform.col[3][2]
		);

	obj->right = vector3(
			obj->transform.col[0][0],
			obj->transform.col[1][0],
			obj->transform.col[2][0]
		);

	obj->up = vector3(
			obj->transform.col[0][1],
			obj->transform.col[1][1],
			obj->transform.col[2][1]
		);

	obj->forward = vector3(
			obj->transform.col[0][2],
			obj->transform.col[1][2],
			obj->transform.col[2][2]
		);

	obj->scale = vector3(
		vec3_normalize(obj->right),
		vec3_normalize(obj->up),
		vec3_normalize(obj->forward)
	);

	obj->is_transform_dirty = false;
}

void obj_update_local_transform(object_t *obj)
{
	if (obj == NULL || !obj->is_local_transform_dirty) {
		return;
	}

	float rx = obj->local_rotation.x;
	float ry = obj->local_rotation.y;
	float rz = obj->local_rotation.z;
	float rw = obj->local_rotation.w;

	float qxx = (rx * rx);
	float qyy = (ry * ry);
	float qzz = (rz * rz);
	float qxz = (rx * rz);
	float qxy = (rx * ry);
	float qyz = (ry * rz);
	float qwx = (rw * rx);
	float qwy = (rw * ry);
	float qwz = (rw * rz);

	mat_set(&obj->local_transform,

		obj->local_scale.x * (1.0f - 2 * (qyy + qzz)),
		obj->local_scale.x * 2 * (qxy + qwz),
		obj->local_scale.x * 2 * (qxz - qwy),
		0,

		obj->local_scale.y * 2 * (qxy - qwz),
		obj->local_scale.y * (1.0f - 2 * (qxx + qzz)),
		obj->local_scale.y * 2 * (qyz + qwx),
		0,

		obj->local_scale.z * 2 * (qxz + qwy),
		obj->local_scale.z * 2 * (qyz - qwx),
		obj->local_scale.z * (1.0f - 2 * (qxx + qyy)),
		0,

		obj->local_position.x,
		obj->local_position.y,
		obj->local_position.z,
		1
	);

	obj->is_local_transform_dirty = false;
}

void obj_update_rotation(object_t *obj)
{
	if (obj == NULL || !obj->is_rotation_dirty) {
		return;
	}

	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	if (obj->parent != NULL) {
		obj->rotation.w = sqrtf(MAX(0, 1 + obj->right.x + obj->up.y + obj->forward.z)) / 2;
		obj->rotation.x = sqrtf(MAX(0, 1 + obj->right.x - obj->up.y - obj->forward.z)) / 2;
		obj->rotation.y = sqrtf(MAX(0, 1 - obj->right.x + obj->up.y - obj->forward.z)) / 2;
		obj->rotation.z = sqrtf(MAX(0, 1 - obj->right.x - obj->up.y + obj->forward.z)) / 2;
	}
	else {
		obj->rotation = obj->local_rotation;
	}

	obj->is_rotation_dirty = false;
}
