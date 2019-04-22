#include "material.h"
#include "core/memory.h"
#include "core/string.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

material_t *material_create(const char *name, const char *path)
{
	NEW(material_t, material);

	material->resource.name = string_duplicate(name);

	// The name of the material is formatted <mtl file name>/<material>.
	// Get the name of the material file without any extensions.
	if (!string_is_null_or_empty(path)) {
	
		char mtl_name[260];
		string_get_file_name_without_extension(path, mtl_name, sizeof(mtl_name));

		char res_name[1000];
		snprintf(res_name, sizeof(res_name), "%s/%s", mtl_name, name);

		material->resource.res_name = string_duplicate(res_name);
		material->resource.path = string_duplicate(path);
	}
	else {
		material->resource.res_name = string_duplicate(name);
	}

	arr_init(material->parameters);

	return material;
}

void material_destroy(material_t *material)
{
	if (material == NULL) {
		return;
	}

	for (uint32_t i = 0; i < material->parameters.count; i++) {
		DESTROY(material->parameters.items[i].name);
	}

	// TODO: Add reference counting to duplicated resources!
	// We're assuming here that a shader without a source code path is a duplicated shader and can
	// be destroyed whenever the material is as well.
	if (string_is_null_or_empty(material->shader->resource.path)) {
		shader_destroy(material->shader);
	}

	arr_clear(material->parameters);

	DESTROY(material->resource.name);
	DESTROY(material->resource.res_name);
	DESTROY(material->resource.path);
	DESTROY(material);
}

void material_apply_parameters(material_t *material)
{
	if (material == NULL || material->shader == NULL) {
		return;
	}

	// Commit each parameter defined in the material to the shader.
	for (uint32_t i = 0; i < material->parameters.count; i++) {

		material_param_t *param = &material->parameters.items[i];

		switch (param->type) {

			case UNIFORM_TYPE_FLOAT:
				shader_set_uniform_float(material->shader, param->name, param->value.f);
				break;

			case UNIFORM_TYPE_INT:
				shader_set_uniform_int(material->shader, param->name, param->value.i);
				break;

			case UNIFORM_TYPE_VECTOR4:
			case UNIFORM_TYPE_COLOUR:
				shader_set_uniform_vector(material->shader, param->name, param->value.vec);
				break;

			default:
				log_warning("Material", "Unhandled material parameter type %d in material %s.",
				            param->type, material->resource.name);
				break;
		}
	}
}
