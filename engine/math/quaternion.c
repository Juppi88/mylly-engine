#include "quaternion.h"
#include "math.h"

// Identity quaternion (zero rotation)
quat_t quat_identity = quat_identity();

// --------------------------------------------------------------------------------

quat_t quat_from_euler(float x, float y, float z)
{
	float sx, cx, sy, cy, sz, cz;
	float sxcy, cxcy, sxsy, cxsy;

	math_sincos(y * 0.5f, &sz, &cz);
	math_sincos(x * 0.5f, &sy, &cy);
	math_sincos(z * 0.5f, &sx, &cx);

	sxcy = sx * cy;
	cxcy = cx * cy;
	sxsy = sx * sy;
	cxsy = cx * sy;

	quat_t quat = quat(
		cxsy * sz - sxcy * cz,
		-cxsy * cz - sxcy * sz,
		sxsy * cz - cxcy * sz,
		cxcy * cz + sxsy * sz
	);

	return quat;
}

quat_t quat_from_euler3(const vec3_t *euler)
{
	return quat_from_euler(euler->x, euler->y, euler->z);
}

vec3_t quat_to_euler(const quat_t *quat)
{
	vec3_t v;

	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;

	x2 = quat->x + quat->x;
	y2 = quat->y + quat->y;
	z2 = quat->z + quat->z;

	xx = quat->x * x2;
	xy = quat->x * y2;
	xz = quat->x * z2;

	yy = quat->y * y2;
	yz = quat->y * z2;
	zz = quat->z * z2;

	wx = quat->w * x2;
	wy = quat->w * y2;
	wz = quat->w * z2;

	float theta, cp, sp;

	sp = xz + wy;
	sp = CLAMP(sp, -1.0f, 1.0f); // Clamp sin value to prevent getting NaN results.

	theta = -asinf(sp);
	cp = cosf(theta);

	if (cp > 8192.0f * FLOAT_EPSILON) {
		v.x	= theta ;
		v.y = atan2f(xy - wz, 1.0f - (yy + zz));
		v.z = atan2f(yz - wx, 1.0f - (xx + yy));
	}
	else {
		v.x	= theta ;
		v.y = -atan2f(xy + wz, 1.0f - (xx + zz));
		v.z = 0;
	}

	return v;
}

quat_t quat_multiply(const quat_t *a, const quat_t *b)
{
	quat_t result = quat(
		a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y,
		a->w * b->y + a->y * b->w + a->z * b->x - a->x * b->z,
		a->w * b->z + a->z * b->w + a->x * b->y - a->y * b->x,
		a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z
	);

	return result;
}

vec3_t quat_multiply_vec3(const quat_t *q, const vec3_t *v)
{
	float xxzz = q->x * q->x - q->z * q->z;
	float wwyy = q->w * q->w - q->y * q->y;

	float xw2 = q->x * q->w * 2.0f;
	float xy2 = q->x * q->y * 2.0f;
	float xz2 = q->x * q->z * 2.0f;
	float yw2 = q->y * q->w * 2.0f;
	float yz2 = q->y * q->z * 2.0f;
	float zw2 = q->z * q->w * 2.0f;

	float x = (xxzz + wwyy) * v->x + (xy2 + zw2) * v->y + (xz2 - yw2) * v->z;
	float y = (xy2 - zw2) * v->x + (q->y * q->y + q->w * q->w - q->x * q->x - q->z * q->z) *
			v->y + (yz2 + xw2) * v->z;
	float z = (xz2 + yw2) * v->x + (yz2 - xw2) * v->y + (wwyy - xxzz) * v->z;

	vec3_t result = vec3(x, y, z);

	return result;
}
