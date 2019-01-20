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
		CLAMP(_r, 0, 255),
		CLAMP(_g, 0, 255),
		CLAMP(_b, 0, 255),
		CLAMP(_a, 0, 255)
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
