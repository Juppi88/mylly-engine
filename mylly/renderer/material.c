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

	arr_clear(material->parameters);

	DESTROY(material->resource.name);
	DESTROY(material->resource.res_name);
	DESTROY(material->resource.path);
	DESTROY(material);
}
