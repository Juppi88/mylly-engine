#include "matrix.h"

mat_t mat_identity = mat_identity();

// --------------------------------------------------------------------------------

void mat_multiply3(const mat_t *mat, const vec3_t *v, vec3_t *out)
{
	out->x = mat->col[0][0] * v->x + mat->col[0][1] * v->y + mat->col[0][2] * v->z + mat->col[0][3];
	out->y = mat->col[1][0] * v->x + mat->col[1][1] * v->y + mat->col[1][2] * v->z + mat->col[1][3];
	out->z = mat->col[2][0] * v->x + mat->col[2][1] * v->y + mat->col[2][2] * v->z + mat->col[2][3];
}

void mat_multiply4(const mat_t *mat, const vec4_t *v, vec4_t *out)
{
	out->x = mat->col[0][0] * v->x + mat->col[0][1] * v->y +
			 mat->col[0][2] * v->z + mat->col[0][3] * v->w;

	out->y = mat->col[1][0] * v->x + mat->col[1][1] * v->y +
			 mat->col[1][2] * v->z + mat->col[1][3] * v->w;

	out->z = mat->col[2][0] * v->x + mat->col[2][1] * v->y +
			 mat->col[2][2] * v->z + mat->col[2][3] * v->w;

	out->w = mat->col[3][0] * v->x + mat->col[3][1] * v->y +
			 mat->col[3][2] * v->z + mat->col[3][3] * v->w;
}

void mat_multiply(const mat_t *mat1, const mat_t *mat2, mat_t *out)
{
	// Get pointers to the matrices as float arrays.
	const float
		*m1 = mat_as_ptr(*mat1),
		*m2 = mat_as_ptr(*mat2);
		
	float *m = mat_as_ptr(*out);

	for (int i = 0; i < 4; i++) {

		for (int j = 0; j < 4; j++) {
			
			 *m = m2[0] * m1[0 * 4 + j]
				+ m2[1] * m1[1 * 4 + j]
				+ m2[2] * m1[2 * 4 + j]
				+ m2[3] * m1[3 * 4 + j];
			++m;
		}

		m2 += 4;
	}
}
