#include "matrix.h"
#include <cglm/cglm.h>

// --------------------------------------------------------------------------------

vec3_t mat_multiply3(mat_t mat, vec3_t v)
{
	vec3_t result;
	glm_mat4_mulv3(mat.col, v.vec, 1, result.vec);
	
	return result;
}

vec4_t mat_multiply4(mat_t mat, vec4_t v)
{
	vec4_t result;
	glm_mat4_mulv(mat.col, v.vec, result.vec);
	
	return result;
}

void mat_multiply(mat_t mat1, mat_t mat2, mat_t *out)
{
	glm_mat4_mul(mat1.col, mat2.col, out->col);
}

quat_t mat_to_quat(mat_t mat)
{
	quat_t quat;
	glm_mat4_quat(mat.col, quat.vec);

	return quat;
}

mat_t mat_invert(mat_t mat)
{
	mat_t dest;
	glm_mat4_inv(mat.col, dest.col);
	return dest;
}
