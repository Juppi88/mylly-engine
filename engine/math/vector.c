#include "vector.h"
#include "math.h"
#include <cglm/cglm.h>

// --------------------------------------------------------------------------------

vec2_t vec2_add(vec2_t a, vec2_t b)
{
	vec2_t result = vec2(
		a.x + b.x,
		a.y + b.y
	);

	return result;
}

vec2_t vec2_subtract(vec2_t a, vec2_t b)
{
	vec2_t result = vec2(
		a.x - b.x,
		a.y - b.y
	);

	return result;
}

vec2_t vec2_multiply(vec2_t v, float value)
{
	vec2_t result = vec2(
		v.x * value,
		v.y * value
	);

	return result;
}

float vec2_dot(vec2_t a, vec2_t b)
{
	return a.x * b.x + a.y * b.y;
}

float vec2_cross(vec2_t a, vec2_t b)
{
	return a.x * b.y + a.y * b.x;
}

float vec2_normalize(vec2_t *v)
{
	float length = sqrtf(v->x * v->x + v->y * v->y);

	if (length != 0) {
		v->x /= length;
		v->y /= length;
	}

	return length;
}

vec2_t vec2_normalized(vec2_t v)
{
	vec2_normalize(&v);
	return v;
}


bool vec2_is_zero(vec2_t v)
{
	return IS_ZERO(v.x) && IS_ZERO(v.y);
}

// --------------------------------------------------------------------------------

vec3_t vec3_add(vec3_t a, vec3_t b)
{
	vec3_t result;
	glm_vec_add(a.vec, b.vec, result.vec);

	return result;
}

vec3_t vec3_subtract(vec3_t a, vec3_t b)
{
	vec3_t result;
	glm_vec_sub(a.vec, b.vec, result.vec);

	return result;
}

vec3_t vec3_multiply(vec3_t v, float value)
{
	vec3_t result;
	glm_vec_scale(v.vec, value, result.vec);

	return result;
}

float vec3_dot(vec3_t a, vec3_t b)
{
	return glm_vec_dot(a.vec, b.vec);
}

vec3_t vec3_cross(vec3_t a, vec3_t b)
{
	vec3_t result;
	glm_vec_cross(a.vec, b.vec, result.vec);

	return result;
}

float vec3_normalize(vec3_t *v)
{
	float length = glm_vec_norm(v->vec);
	glm_vec_divs(v->vec, length, v->vec);

	return length;
}

vec3_t vec3_normalized(vec3_t v)
{
	vec3_t result;

	glm_vec_copy(v.vec, result.vec);
	glm_vec_normalize(result.vec);

	return result;
}

void vec3_orthonormalize(vec3_t *a, vec3_t *b)
{
	vec3_normalize(a);
	vec3_t v = vec3_cross(*a, *b);
	vec3_normalize(&v);

	*b = vec3_cross(v, *a);
}

vec3_t vec3_sanitize_rotation(vec3_t v)
{
	vec3_t vec = vec3(
		math_sanitize_angle(v.x),
		math_sanitize_angle(v.y),
		math_sanitize_angle(v.z)
	);

	return vec;
}

// --------------------------------------------------------------------------------

float vec4_normalize(vec4_t v)
{
	float length = glm_vec4_norm(v.vec);
	glm_vec4_divs(v.vec, length, v.vec);

	return length;
}

vec4_t vec4_normalized(vec4_t v)
{
	vec4_t result;

	glm_vec4_copy(v.vec, result.vec);
	glm_vec4_normalize(result.vec);

	return result;
}
