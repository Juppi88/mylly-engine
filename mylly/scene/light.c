#include "light.h"
#include "core/memory.h"
#include "collections/array.h"

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

	light->angle = 1.0f;
	light->direction = vec3_up();

	return light;
}

void light_destroy(light_t *light)
{
	if (light == NULL) {
		return;
	}

	DESTROY(light);
}
