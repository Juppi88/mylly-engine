#include "shader.h"
#include "renderer.h"
#include "core/memory.h"
#include "core/string.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

static void shader_destroy_program(shader_t *shader);
static void shader_add_uniform(shader_t *shader, const char *name, UNIFORM_TYPE type);
static shader_uniform_t *shader_get_uniform(shader_t *shader, const char *name);

// -------------------------------------------------------------------------------------------------

// Shader vertex attribute names.
static const char *shader_attribute_names[NUM_SHADER_ATTRIBUTES] = {
	"Vertex",
	"Normal",
	"TexCoord",
	"Colour",
	"ParticleCentre",
	"ParticleEmitPosition",
	"ParticleRotation",
	"ParticleSize",
	"Tangent"
};

// Shader uniform array names.
#define MATRIX_ARRAY_NAME "MatrixArr"
#define VECTOR_ARRAY_NAME "VectorArr"
#define SAMPLER_ARRAY_NAME "SamplerArr"
#define LIGHT_ARRAY_NAME "LightArr"
#define NUM_LIGHTS_NAME "NumLights"

// -------------------------------------------------------------------------------------------------

shader_t *shader_create(const char *name, const char *path)
{
	NEW(shader_t, shader);

	shader->resource.res_name = string_duplicate(name);
	shader->resource.name = shader->resource.res_name;

	if (path != NULL) {
		shader->resource.path = string_duplicate(path);
	}

	shader->vertex = 0;
	shader->fragment = 0;
	shader->program = 0;

	shader->matrix_array = -1;
	shader->vector_array = -1;
	shader->sampler_array = -1;

	shader->light_array = -1;
	shader->num_lights_position = -1;

	arr_init(shader->material_uniforms);
	arr_init(shader->source);

	return shader;
}

shader_t *shader_clone(shader_t *original)
{
	if (original == NULL) {
		return NULL;
	}

	shader_t *shader = shader_create(original->resource.res_name, NULL);

	// Compile the shader program.
	if (shader_load_from_source(shader, original->source.count, (const char **)original->source.items,
	                            0, NULL, NULL)) {

		// Resolve custom material uniforms.
		for (size_t i = 0; i < original->material_uniforms.count; i++) {

			shader_uniform_t *uniform = &original->material_uniforms.items[i];
			shader_add_uniform(shader, uniform->name, uniform->type);
		}
	}
	
	// TODO: Add reference counting to duplicated resources!
	return shader;
}

void shader_destroy(shader_t *shader)
{
	if (shader == NULL) {
		return;
	}

	// Destroy the GPU objects.
	shader_destroy_program(shader);

	for (uint32_t i = 0; i < shader->material_uniforms.count; i++) {
		DESTROY(shader->material_uniforms.items[i].name);
	}

	arr_clear(shader->material_uniforms);

	DESTROY(shader->resource.res_name);
	DESTROY(shader->resource.path);
	DESTROY(shader);
}

void shader_set_uniform_int(shader_t *shader, const char *name, int value)
{
	if (shader == NULL || string_is_null_or_empty(name)) {
		return;
	}

	shader_uniform_t *uniform = shader_get_uniform(shader, name);

	if (uniform == NULL) {
		return;
	}

	if (uniform->type != UNIFORM_TYPE_INT) {

		log_warning("Shader", "Uniform '%s' in shader %s is not of type int.",
			name, shader->resource.name);

		return;
	}

	// Update the value and flag the shader to be updated before rendering the next frame.
	uniform->value.i = value;
	shader->has_updated_uniforms = true;
}

void shader_set_uniform_float(shader_t *shader, const char *name, float value)
{
	if (shader == NULL || string_is_null_or_empty(name)) {
		return;
	}

	shader_uniform_t *uniform = shader_get_uniform(shader, name);

	if (uniform == NULL) {
		return;
	}

	if (uniform->type != UNIFORM_TYPE_FLOAT) {

		log_warning("Shader", "Uniform '%s' in shader %s is not of type float.",
			name, shader->resource.name);

		return;
	}

	// Update the value and flag the shader to be updated before rendering the next frame.
	uniform->value.f = value;
	shader->has_updated_uniforms = true;
}

void shader_set_uniform_vector(shader_t *shader, const char *name, vec4_t value)
{
	if (shader == NULL || string_is_null_or_empty(name)) {
		return;
	}

	shader_uniform_t *uniform = shader_get_uniform(shader, name);

	if (uniform == NULL) {
		return;
	}

	if (uniform->type != UNIFORM_TYPE_VECTOR4) {

		log_warning("Shader", "Uniform '%s' in shader %s is not of type vec4.",
			name, shader->resource.name);

		return;
	}

	// Update the value and flag the shader to be updated before rendering the next frame.
	uniform->value.vec = value;
	shader->has_updated_uniforms = true;
}

void shader_set_uniform_colour(shader_t *shader, const char *name, colour_t value)
{
	// Convenience method for setting a colour to a vec4 type uniform field.
	shader_set_uniform_vector(shader, name, col_to_vec4(value));
}

void shader_set_render_queue(shader_t *shader, SHADER_QUEUE queue)
{
	if (shader == NULL || queue < 0 || queue >= NUM_QUEUES) {
		return;
	}

	shader->queue = queue;
}

bool shader_load_from_source(
	shader_t *shader,
	size_t num_lines, const char **lines,
	size_t num_uniforms, const char **uniforms, UNIFORM_TYPE *uniform_types)
{
	if (shader == NULL) {
		return false;
	}

	// Destroy previously created program first.
	shader_destroy_program(shader);

	const char *log;

	// Compile the vertex shader.
	shader->vertex = rend_create_shader(SHADER_VERTEX, lines, num_lines, &log);

	if (shader->vertex == 0) {

		log_warning("Renderer", "Failed to compile vertex shader '%s':\n%s",
			shader->resource.res_name, log);
		
		return false;
	}

	// Compile the fragment shader.
	shader->fragment = rend_create_shader(SHADER_FRAGMENT, lines, num_lines, &log);

	if (shader->fragment == 0) {

		log_warning("Renderer", "Failed to compile fragment shader '%s':\n%s",
			shader->resource.res_name, log);

		return false;
	}

	// Link the shader objects into a shader program.
	shader_object_t shaders[2] = { shader->vertex, shader->fragment };

	shader->program = rend_create_shader_program(shaders, 2);

	if (shader->program == 0) {

		log_warning("Renderer", "Failed to link shader program '%s'", shader->resource.res_name);
		return false;
	}

	// Get and cache shader uniform locations.
	shader->matrix_array = rend_get_program_uniform_location(shader->program, MATRIX_ARRAY_NAME);
	shader->vector_array = rend_get_program_uniform_location(shader->program, VECTOR_ARRAY_NAME);
	shader->sampler_array = rend_get_program_uniform_location(shader->program, SAMPLER_ARRAY_NAME);

	shader->light_array = rend_get_program_uniform_location(shader->program, LIGHT_ARRAY_NAME);
	shader->num_lights_position = rend_get_program_uniform_location(shader->program, NUM_LIGHTS_NAME);

	// Custom material uniforms.
	for (size_t i = 0; i < num_uniforms; i++) {
		shader_add_uniform(shader, uniforms[i], uniform_types[i]);
	}

	// Get and cache vertex attribute indices.
	for (uint32_t i = 0; i < NUM_SHADER_ATTRIBUTES; ++i) {

		int attribute = rend_get_program_program_attribute_location(
			shader->program,
			shader_attribute_names[i]
		);

		shader->attributes[i] = attribute;
	}

	// Resolve the render queue used by the shader. If the shader doesn't define its queue, use
	// the geometry queue by default.
	shader->queue = QUEUE_GEOMETRY;

	for (uint32_t i = 0; i < num_lines; ++i) {

		if (lines[i] == NULL) {
			continue;
		}

		if (string_starts_with(lines[i], "#pragma queue ", 14)) {

			const char *queue = &lines[i][14];

			if (string_starts_with(queue, "BACKGROUND", 10)) shader->queue = QUEUE_BACKGROUND;
			else if (string_starts_with(queue, "GEOMETRY", 8)) shader->queue = QUEUE_GEOMETRY;
			else if (string_starts_with(queue, "TRANSPARENT", 11)) shader->queue = QUEUE_TRANSPARENT;
			else if (string_starts_with(queue, "OVERLAY", 7)) shader->queue = QUEUE_OVERLAY;

			break;
		}
	}

	// Store shader source code for shader duplicating.
	for (uint32_t i = 0; i < num_lines; i++) {
		arr_push(shader->source, string_duplicate(lines[i]));
	}

	return true;
}

static void shader_destroy_program(shader_t *shader)
{
	if (shader == NULL) {
		return;
	}

	if (shader->vertex != 0) {

		rend_destroy_shader(shader->vertex);
		shader->vertex = 0;
	}

	if (shader->fragment != 0) {

		rend_destroy_shader(shader->fragment);
		shader->fragment = 0;
	}

	if (shader->program != 0) {

		rend_destroy_shader(shader->program);
		shader->program = 0;
	}

	// Destroy source code storage.
	const char *src_line;

	arr_foreach(shader->source, src_line) {
		DESTROY(src_line);
	}

	arr_clear(shader->source);
}

static void shader_add_uniform(shader_t *shader, const char *name, UNIFORM_TYPE type)
{
	if (shader == NULL || string_is_null_or_empty(name)) {
		return;
	}

	int position = rend_get_program_uniform_location(shader->program, name);

	if (position < 0) {

		// If the uniform has no position in the compiled program, it is not used and the
		// compiler has optimized it away. Ignore the uniform.
		return;
	}

	// Store the uniform into the shader.
	shader_uniform_t uniform;

	uniform.name = string_duplicate(name);
	uniform.type = type;
	uniform.position = position;

	// Initialize value to 0.
	memset(&uniform.value, 0, sizeof(uniform.value));

	arr_push(shader->material_uniforms, uniform);
}

static shader_uniform_t *shader_get_uniform(shader_t *shader, const char *name)
{
	for (uint32_t i = 0; i < shader->material_uniforms.count; i++) {

		shader_uniform_t *uniform = &shader->material_uniforms.items[i];

		if (string_equals(uniform->name, name)) {
			return uniform;
		}
	}

	log_warning("Shader", "Uniform '%s' does not exist in shader %s.", name, shader->resource.name);
	return NULL;
}
