#include "shaderdata.h"
#include "core/string.h"
#include "collections/array.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

static arr_t(shader_uniform_t) uniforms;

// -------------------------------------------------------------------------------------------------

void shader_data_initialize(void)
{
	arr_init(uniforms);

	// Create definitions for built-in, material independent uniforms.
	shader_data_add_uniform("MatrixModel", DATATYPE_MATRIX); // 0
	shader_data_add_uniform("MatrixView", DATATYPE_MATRIX); // 1
	shader_data_add_uniform("MatrixProjection", DATATYPE_MATRIX); // 2
	shader_data_add_uniform("MatrixMVP", DATATYPE_MATRIX); // 3
	shader_data_add_uniform("ViewPosition", DATATYPE_VECTOR4); // 4
	shader_data_add_uniform("Texture", DATATYPE_INT); // 5
	shader_data_add_uniform("Time", DATATYPE_VECTOR4); // 6
	shader_data_add_uniform("Screen", DATATYPE_VECTOR4); // 7
}

void shader_data_shutdown(void)
{
	for (size_t i = 0; i < uniforms.count; i++) {
		DESTROY(uniforms.items[i].name);
	}

	arr_clear(uniforms);
}

uint32_t shader_data_get_uniform_count(void)
{
	return uniforms.count;
}

uint32_t shader_data_add_uniform(const char *name, shader_data_type_t type)
{
	// If a uniform by that name has already been defined, return its index.
	uint32_t index;

	if (shader_data_uniform_exists(name, &index)) {
		return index;
	}

	shader_uniform_t data = {
		uniforms.count, // Index
		string_duplicate(name), // Name
		type // Data type
	};

	// Add a new uniform definition.
	arr_push(uniforms, data);

	return data.index;
}

const shader_uniform_t *shader_data_get_uniform_by_index(uint32_t index)
{
	if (index >= uniforms.count) {

		log_warning("Shader", "Could not find shader uniform by index %u.", index);
		return NULL;
	}

	return &uniforms.items[index];
}

bool shader_data_uniform_exists(const char *name, uint32_t *out_index)
{
	for (size_t i = 0; i < uniforms.count; i++) {

		if (string_equals(uniforms.items[i].name, name)) {

			if (out_index != NULL) {
				*out_index = i;
			}
			
			return true;
		}
	}

	return false;
}
