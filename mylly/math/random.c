#include "random.h"
#include <math.h>

// -------------------------------------------------------------------------------------------------

vec3_t random_point_on_shpere(void)
{
	float theta = 2 * PI * randomf(0, 1);
	float phi = acosf(1 - 2 * randomf(0, 1));

	return vec3(
		sinf(phi) * cosf(theta),
		sinf(phi) * sinf(theta),
		cosf(phi)
	);
}

vec3_t random_point_on_circle(void)
{
	float theta = 2 * PI * randomf(0, 1);

	return vec3(
		cosf(theta),
		sinf(theta),
		0
	);
}

vec3_t random_point_on_cone(float angle)
{
	float theta = 2 * PI * randomf(0, 1);
	float phi = angle;

	return vec3(
		sinf(phi) * cosf(theta),
		cosf(phi),
		sinf(phi) * sinf(theta)
	);
}
