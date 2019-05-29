#include "colour.h"
#include "math/math.h"

// -------------------------------------------------------------------------------------------------

colour_t col_hsv(float h, float s, float v)
{
	float r = 0, g = 0, b = 0;

	h *= 360.0f;

	if (s == 0) {
		r = v;
		g = v;
		b = v;
	}
	else {

		if (h == 360) {
			h = 0;
		}
		else {
			h /= 60;
		}

		int i = (int)trunc(h);
		float f = h - i;

		float p = v * (1.0f - s);
		float q = v * (1.0f - (s * f));
		float t = v * (1.0f - (s * (1.0f - f)));

		switch (i) {

		case 0:
			r = v;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = v;
			b = p;
			break;

		case 2:
			r = p;
			g = v;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = v;
			break;

		case 4:
			r = t;
			g = p;
			b = v;
			break;

		default:
			r = v;
			g = p;
			b = q;
			break;
		}
	}

	colour_t colour = col(
		(uint8_t)(r * 255),
		(uint8_t)(g * 255),
		(uint8_t)(b * 255)
	);

	return colour;
}

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
