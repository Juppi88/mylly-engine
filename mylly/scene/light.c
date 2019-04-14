#include "light.h"
#include "object.h"
#include "core/memory.h"
#include "collections/array.h"
#include "math/math.h"

// -------------------------------------------------------------------------------------------------

static void light_update_shader_params(light_t *light);

// -------------------------------------------------------------------------------------------------

light_t *light_create(object_t *parent)
{
	NEW(light_t, light);

	light->parent = parent;
	light->scene_index = INVALID_INDEX;

	light->type = LIGHT_DIRECTIONAL;
	light->colour = COL_WHITE;
	light->range = 1.0f;
	light->intensity = 1.0f;

	light->cutoff_angle = 0;
	light->cutoff_angle_outer = 0;
	light->direction = vec3_up();

	light->is_dirty = true;

	return light;
}

void light_destroy(light_t *light)
{
	if (light == NULL) {
		return;
	}

	DESTROY(light);
}

void light_set_type(light_t *light, light_type_t type)
{
	if (light == NULL) {
		return;
	}

	light->type = type;
	light->is_dirty = true;

	// Set light cutoff angle to zero for all non-spotlights.
	if (type != LIGHT_SPOT) {
		light->cutoff_angle = 0;
		light->cutoff_angle_outer = 0;
	}
}

void light_set_colour(light_t *light, colour_t colour)
{
	if (light == NULL) {
		return;
	}

	light->colour = colour;
	light->is_dirty = true;
}

void light_set_intensity(light_t *light, float intensity)
{
	if (light == NULL) {
		return;
	}

	light->intensity = intensity;
	light->is_dirty = true;
}

void light_set_range(light_t *light, float range)
{
	if (light == NULL) {
		return;
	}

	light->range = range;
	light->is_dirty = true;
}

void light_set_spotlight_cutoff_angle(light_t *light, float angle, float outer_angle)
{
	if (light == NULL) {
		return;
	}

	light->cutoff_angle = cosf(DEG_TO_RAD(angle));
	light->cutoff_angle_outer = cosf(DEG_TO_RAD(outer_angle));
	light->is_dirty = true;
}

void light_set_spotlight_direction(light_t *light, vec3_t direction)
{
	if (light == NULL) {
		return;
	}

	light->direction = direction;
	light->is_dirty = true;
}

mat_t light_get_shader_params(light_t *light)
{
	if (light == NULL) {
		return mat_identity();
	}

	// Ensure parameters are always up to date.
	if (light->is_dirty) {
		light_update_shader_params(light);
	}

	return light->shader_params;
}

static void light_update_shader_params(light_t *light)
{
	// Encode light parameters into a matrix.
	vec3_t position = (light->type != LIGHT_DIRECTIONAL ?
	                   obj_get_position(light->parent) :
	                   light->direction);

	light->shader_params.col[0][0] = position.x;
	light->shader_params.col[0][1] = position.y;
	light->shader_params.col[0][2] = position.z;
	light->shader_params.col[0][3] = (light->type == LIGHT_DIRECTIONAL ? 0 : 1);

	light->shader_params.col[1][0] = light->colour.r / 255.0f;
	light->shader_params.col[1][1] = light->colour.g / 255.0f;
	light->shader_params.col[1][2] = light->colour.b / 255.0f;
	light->shader_params.col[1][3] = 1.0f;

	light->shader_params.col[2][0] = light->direction.x;
	light->shader_params.col[2][1] = light->direction.y;
	light->shader_params.col[2][2] = light->direction.z;
	light->shader_params.col[2][3] = 0.0f;

	light->shader_params.col[3][0] = light->range;
	light->shader_params.col[3][1] = light->intensity;
	light->shader_params.col[3][2] = (light->type == LIGHT_SPOT ? light->cutoff_angle : 0);
	light->shader_params.col[3][3] = (light->type == LIGHT_SPOT ? light->cutoff_angle_outer : 0);

	light->is_dirty = false;
}
