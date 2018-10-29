#include "vector.h"
#include "math.h"
#include <cglm/cglm.h>

// --------------------------------------------------------------------------------

vec2_t vec2_zero = vec2_zero();
vec2_t vec2_one = vec2_one();

vec3_t vec3_zero = vec3_zero();
vec3_t vec3_one = vec3_one();
vec3_t vec3_right = vec3(1, 0, 0);
vec3_t vec3_up = vec3(0, 1, 0);
vec3_t vec3_forward = vec3(0, 0, 1);

vec4_t vec4_zero = vec4_zero();
vec4_t vec4_one = vec4_one();

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

float vec3_normalize(vec3_t v)
{
	float length = glm_vec_norm(v.vec);
	glm_vec_divs(v.vec, length, v.vec);

	return length;
}

vec3_t vec3_normalized(vec3_t v)
{
	vec3_t result;

	glm_vec_copy(v.vec, result.vec);
	glm_vec_normalize(result.vec);

	return result;
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
