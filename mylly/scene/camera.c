#include "camera.h"
#include "object.h"
#include "io/log.h"
#include "math/math.h"
#include "core/mylly.h"

// --------------------------------------------------------------------------------

camera_t *camera_create(object_t *parent)
{
	NEW(camera_t, camera);

	camera->parent = parent;
	camera->scene_index = INVALID_INDEX;

	// Flag all matrices dirty at first.
	camera->state = (
		CAMSTATE_VIEW_DIRTY |
		CAMSTATE_PROJ_DIRTY |
		CAMSTATE_VIEWPROJ_DIRTY |
		CAMSTATE_VIEWPROJ_INV_DIRTY
	);

	camera->is_orthographic = true;
	camera->clip_near = ORTOGRAPHIC_NEAR;
	camera->clip_far = ORTOGRAPHIC_FAR;
	camera->size = 2;
	camera->fov = 60;

	return camera;
}

void camera_destroy(camera_t *camera)
{
	if (camera == NULL) {
		return;
	}

	DESTROY(camera);
}

void camera_set_orthographic_projection(camera_t *camera, float size, float near, float far)
{
	if (camera == NULL) {
		return;
	}

	// Set camera parameters for orthographic projection.
	camera->is_orthographic = true;
	camera->size = size;
	camera->clip_near = near;
	camera->clip_far = far;

	// Flag the camera as dirty so the projection matrix is recalculated before use.
	camera->state |= (CAMSTATE_PROJ_DIRTY | CAMSTATE_VIEWPROJ_DIRTY | CAMSTATE_VIEWPROJ_INV_DIRTY);
}

void camera_set_perspective_projection(camera_t *camera, float fov, float near, float far)
{
	if (camera == NULL) {
		return;
	}

	// Set camera parameters for perspective projection.
	camera->is_orthographic = false;
	camera->fov = fov;
	camera->clip_near = near;
	camera->clip_far = far;

	// Flag the camera as dirty so the projection matrix is recalculated before use.
	camera->state |= (CAMSTATE_PROJ_DIRTY | CAMSTATE_VIEWPROJ_DIRTY | CAMSTATE_VIEWPROJ_INV_DIRTY);
}

vec3_t camera_world_to_screen(camera_t *camera, vec3_t position)
{
	if (camera == NULL) {
		return vec3_zero();
	}

	vec4_t projected = mat_multiply4(
		*camera_get_view_projection_matrix(camera),
		vec3_to_vec4(position)
	);

	// The projected position is in normalized -1...1 coordinates, translate them to screen units.
	// Also calculate and return the depth value as the z-coordinate.
	uint16_t screen_width, screen_height;
	mylly_get_resolution(&screen_width, &screen_height);

	return vec3(
		screen_width * (projected.x + 1.0f) / 2.0f,
		screen_height * (1.0f - (projected.y + 1.0f) / 2.0f),
		projected.z / projected.w
	);
}

vec3_t camera_screen_to_world(camera_t *camera, vec3_t position)
{
	if (camera == NULL) {
		return vec3_zero();
	}

	// Calculate depth (and screen position) at the camera's position, moved position.z units away
	// towards the forward vector.
	vec3_t direction = vec3_multiply(obj_get_forward_vector(camera->parent), position.z);
	vec3_t target_depth_pos = vec3_add(obj_get_position(camera->parent), direction);

	vec3_t target_depth_screen = camera_world_to_screen(camera, target_depth_pos);

	// Normalize screen coordinates to a -1...1 range.
	uint16_t screen_width, screen_height;
	mylly_get_resolution(&screen_width, &screen_height);

	vec4_t normalized = vec4(
		2.0f * position.x / screen_width - 1.0f,
		2.0f * (1.0f - position.y / screen_height) - 1.0f,
		target_depth_screen.z,
		1.0f
	);

	// Calculate world position at the screen position and desired depth.
	vec4_t world = mat_multiply4(*camera_get_view_projection_matrix_inverse(camera), normalized);

	// Take into account the perspective projection.
	return vec3(world.x / world.w, world.y / world.w, world.z / world.w);
}

void camera_update_view_matrix(camera_t *camera)
{
	if (camera == NULL || (camera->state & CAMSTATE_VIEW_DIRTY) == 0) {
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

	camera->state &= ~CAMSTATE_VIEW_DIRTY;
}

void camera_update_projection_matrix(camera_t *camera)
{
	if (camera == NULL || (camera->state & CAMSTATE_PROJ_DIRTY) == 0) {
		return;
	}

	// Calculate aspect ratio from the used resolution.
	uint16_t screen_width, screen_height;
	mylly_get_resolution(&screen_width, &screen_height);

	float aspect = (float)screen_width / screen_height;

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
			2.0f / (camera->clip_far - camera->clip_near),
			0,

			-(right + left) / (right - left),
			-(top + bottom) / (top - bottom),
			-(camera->clip_far + camera->clip_near) / (camera->clip_far - camera->clip_near),
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
			(camera->clip_far + camera->clip_near) / (camera->clip_far - camera->clip_near),
			1,

			0,
			0,
			(2 * camera->clip_far * camera->clip_near) / (camera->clip_far - camera->clip_near),
			0
		);
	}

	mat_print(camera->projection);

	camera->state &= ~CAMSTATE_PROJ_DIRTY;
}

void camera_update_view_projection_matrix(camera_t *camera)
{
	if (camera == NULL || (camera->state & CAMSTATE_VIEWPROJ_DIRTY) == 0) {
		return;
	}

	mat_multiply(
		*camera_get_projection_matrix(camera),
		*camera_get_view_matrix(camera),
		&camera->view_projection
	);

	camera->state &= ~CAMSTATE_VIEWPROJ_DIRTY;
}

void camera_update_view_projection_matrix_inverse(camera_t *camera)
{
	if (camera == NULL || (camera->state & CAMSTATE_VIEWPROJ_INV_DIRTY) == 0) {
		return;
	}

	camera->view_projection_inv = mat_invert(*camera_get_view_projection_matrix(camera));
	camera->state &= ~CAMSTATE_VIEWPROJ_INV_DIRTY;
}
