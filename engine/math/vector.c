#include "vector.h"
#include "math.h"

// --------------------------------------------------------------------------------

vec2_t vec2_zero = vec2_zero();
vec2_t vec2_one = vec2_one();

vec3_t vec3_zero = vec3_zero();
vec3_t vec3_one = vec3_one();

vec4_t vec4_zero = vec4_zero();
vec4_t vec4_one = vec4_one();

// --------------------------------------------------------------------------------

float vec3_dot(const vec3_t *a, const vec3_t *b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

vec3_t vec3_cross(const vec3_t *a, const vec3_t *b)
{
	vec3_t cross = vec3(
		a->y * b->z - a->z * b->y,
		a->z * b->x - a->x * b->z,
		a->x * b->y - a->y * b->x
	);

	return cross;
}

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
