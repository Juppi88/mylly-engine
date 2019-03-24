#include "colour.h"
#include "math/math.h"

// -------------------------------------------------------------------------------------------------

colour_t col_add(colour_t a, colour_t b)
{
	uint32_t _r = a.r + b.r;
	uint32_t _g = a.g + b.g;
	uint32_t _b = a.b + b.b;
	uint32_t _a = a.a + b.a;

	return col_a(
		MIN(_r, 255),
		MIN(_g, 255),
		MIN(_b, 255),
		MIN(_a, 255)
	);
}

colour_t col_multiply(colour_t a, colour_t b)
{
	uint32_t _r = (a.r * b.r) / 255;
	uint32_t _g = (a.g * b.g) / 255;
	uint32_t _b = (a.b * b.b) / 255;
	uint32_t _a = (a.a * b.a) / 255;

	return col_a(_r, _g, _b, _a);
}

colour_t col_lerp(colour_t a, colour_t b, float t)
{
	return col_a(
		(uint8_t)lerpi(a.r, b.r, t),
		(uint8_t)lerpi(a.g, b.g, t),
		(uint8_t)lerpi(a.b, b.b, t),
		(uint8_t)lerpi(a.a, b.a, t)
	);
}