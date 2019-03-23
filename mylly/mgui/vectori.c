#include "vectori.h"

vec2i_t vec2i_add(vec2i_t a, vec2i_t b)
{
	return vec2i(a.x + b.x, a.y + b.y);
}

vec2i_t vec2i_subtract(vec2i_t a, vec2i_t b)
{
	return vec2i(a.x - b.x, a.y - b.y);
}

vec2i_t vec2i_multiply(vec2i_t v, float value)
{
	return vec2i(
		(int16_t)(value * v.x),
		(int16_t)(value * v.y)
	);
}
