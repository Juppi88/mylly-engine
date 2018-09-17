#include "quaternion.h"
#include "math.h"

// Identity quaternion
quat_t quat_identity = quat_identity();

// --------------------------------------------------------------------------------

quat_t quat_from_euler(float x, float y, float z)
{
	quat_t quat;
	float sx, cx, sy, cy, sz, cz;

	math_sincos(y * 0.5f, &sx, &cx);
	math_sincos(z * 0.5f, &sy, &cy);
	math_sincos(x * 0.5f, &sz, &cz);

	quat.w = cx * cy * cz - sx * sy * sz;
	quat.x = sx * sy * cz + cx * cy * sz;
	quat.y = sx * cy * cz + cx * sy * sz;
	quat.z = cx * sy * cz - sx * cy * sz;

	return quat;
}

quat_t quat_from_euler3(const vec3_t *euler)
{
	return quat_from_euler(euler->x, euler->y, euler->z);
}
