#include "quaternion.h"
#include "math.h"
#include <cglm/cglm.h>

// --------------------------------------------------------------------------------

quat_t quat_from_euler(float x, float y, float z)
{
	vec3_t euler = vec3(x, y, z);

	// Convert the euler angles to a rotation matrix.
	mat4 rotation;
	glm_euler(euler.vec, rotation);

	// Convert rotation matrix to a quaternion.
	quat_t quat;
	glm_mat4_quat(rotation, quat.vec);

	return quat;
}

quat_t quat_from_euler_deg(float x, float y, float z)
{
	return quat_from_euler(DEG_TO_RAD(x), DEG_TO_RAD(y), DEG_TO_RAD(z));
}

vec3_t quat_to_euler(quat_t quat)
{
	// Convert the quaternion to a rotation matrix.
	mat4 rotation;
	glm_quat_mat4(quat.vec, rotation);

	// Extract euler angles from the rotation matrix.
	vec3_t euler;
	glm_euler_angles(rotation, euler.vec);

	return euler;
}

quat_t quat_multiply(quat_t a, quat_t b)
{
	quat_t result;
	glm_quat_mul(a.vec, b.vec, result.vec);

	return result;
}

vec3_t quat_rotate_vec3(quat_t q, vec3_t v)
{
	vec3_t result;
	glm_quat_rotatev(q.vec, v.vec, result.vec);

	return result;
}
