#include "vector.h"
#include "math.h"

float vec3_normalize(vec3_t *v)
{
	float len2 = (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
	float len = math_sqrt(len2);

	v->x /= len;
	v->y /= len;
	v->z /= len;

	return len;
}

vec3_t vec3_normalized(const vec3_t *v)
{
	vec3_t tmp = *v;
	vec3_normalize(&tmp);

	return tmp;
}

float vec4_normalize(vec4_t *v)
{
	float len2 = (v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w);
	float len = math_sqrt(len2);

	v->x /= len;
	v->y /= len;
	v->z /= len;
	v->w /= len;

	return len;
}

vec4_t vec4_normalized(const vec4_t *v)
{
	vec4_t tmp = *v;
	vec4_normalize(&tmp);

	return tmp;
}
