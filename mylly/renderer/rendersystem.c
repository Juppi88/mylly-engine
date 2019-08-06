#include "rendersystem.h"
#include "renderer.h"
#include "renderview.h"
#include "shader.h"
#include "texture.h"
#include "buffercache.h"
#include "material.h"
#include "debug.h"
#include "scene/scene.h"
#include "scene/object.h"
#include "scene/model.h"
#include "scene/sprite.h"
#include "scene/emitter.h"
#include "scene/camera.h"
#include "scene/light.h"
#include "resources/resources.h"
#include "core/mylly.h"
#include "io/log.h"
#include "math/math.h"
#include "mgui/mgui.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

static int frames_rendered; // Number of frames rendered so far
static shader_t *default_shader; // Default shader used for rendering when a mesh has no shader

static arr_t(rlight_t*) lights; // All the lights affecting the scene
static list_t(rview_t) views; // List of views to be rendered this frame

static rview_t *ui_view; // A dedicated view for UI widgets due to UI not needing a camera
static robject_t ui_parent; // A virtual object to be used as the UI parent

static bool is_using_deferred_lighting = false; // Toggle for forward/deferred lighting mode

// -------------------------------------------------------------------------------------------------

static void rsys_cull_object(object_t *object);
static void rsys_cull_object_meshes(object_t *object, robject_t *parent, rview_t *view);
static void rsys_add_mesh_to_view(mesh_t *mesh, object_t *object, robject_t *parent, rview_t *view);
static rmesh_t *rsys_create_render_mesh(mesh_t *mesh, robject_t *root);
static void rsys_collect_mesh_lights(rmesh_t *render_mesh, object_t *parent);
static int rsys_sort_lights_by_distance(const void *p1, const void *p2);
static void rsys_free_frame_data(void);

// -------------------------------------------------------------------------------------------------

void rsys_initialize(void)
{
	// Initialize renderer backend.
	rend_initialize();
	bufcache_initialize();

	// Initialize UI parent objects.
	ui_parent.matrix = mat_identity();

	// Calculate an MVP matrix for the UI based on screen resolution.
	// TODO: Update this if resolution changes!
	uint16_t screen_width, screen_height;
	mylly_get_resolution(&screen_width, &screen_height);

	mat_set(&ui_parent.mvp,
		2.0f / screen_width, 0, 0, 0,
		0, 2.0f / screen_height, 0, 0,
		0, 0, 1, 0,
		-1, -1, 0, 1
	);

	is_using_deferred_lighting = mylly_get_parameters()->renderer.use_deferred_lighting;

	log_message("RenderSystem", "Rendering system initialized.");
}

void rsys_shutdown(void)
{
	bufcache_shutdown();
	rend_shutdown();
}

void rsys_begin_frame(void)
{
	// Attempt to load the default shader if it has not been loaded.
	if (default_shader == NULL) {
		default_shader = res_get_shader("default");
	}

	arr_init(lights);

	rend_begin_draw();
	debug_begin_frame();

	// Setup a per-frame view object for the UI.
	NEW(rview_t, view);

	ui_view = view;
	ui_view->ambient_light = col_to_vec4(COL_WHITE);
	ui_view->near = ORTOGRAPHIC_NEAR;
	ui_view->far = ORTOGRAPHIC_FAR;

	// Update UI render area resolution.
	ui_parent.mvp.col[0][0] = 2.0f / mgui_parameters.width;
	ui_parent.mvp.col[1][1] = 2.0f / mgui_parameters.height;

	mat_cpy(&ui_view->view_projection, &ui_parent.mvp);
	ui_view->view_position = vec4(0, 0, 0, 1);

	ui_view->view_inv = mat_invert(ui_view->view);
	ui_view->projection_inv = mat_invert(ui_view->projection);
	ui_view->view_projection_inv = mat_invert(ui_view->view_projection);
}

void rsys_end_frame(scene_t *scene)
{
	// Generate and issue the actual draw calls here. All scenes and their objects ahave already
	// been added to the views, soo all that's left are special meshes (debug primitives,
	// UI widgets).

	// First collect debug primitives. We're currently using the first camera for debug drawing.
	if (scene != NULL && scene->cameras.count != 0) {
		debug_end_frame(scene->cameras.items[0]->camera);	
	}
	
	// Add the UI view to the view list as last.
	list_push(views, ui_view);

	// Finalize by issuing the render views to the renderer backend.
	rend_draw_views(views.first);
	rend_end_draw();

	// Release all temporary data.
	rsys_free_frame_data();

	++frames_rendered;
}

void rsys_render_scene(scene_t *scene)
{
	if (scene == NULL) {
		return;
	}

	// Collect info about the objects in the scene before rendering anything and process culling etc.
	// TODO: Also use a proper temp allocator because this is alloc heavy!

	// Collect all the lights affecting the scene.
	object_t *light;

	arr_foreach(scene->lights, light) {

		if (light == NULL || !light->is_active) {
			continue;
		}

		NEW(rlight_t, render_light);

		render_light->light = light->light;

		// Update and create a copy of light shader parameters for this frame.
		light_get_shader_params(light->light);
		mat_cpy(&render_light->shader_params, &light->light->shader_params);

		arr_push(lights, render_light);
	}

	// Create a separate render view for every camera in the scene.
	object_t *camera;

	arr_foreach(scene->cameras, camera) {

		if (camera == NULL || !camera->is_active) {
			continue;
		}

		NEW(rview_t, view);

		// Copy camera matrices.
		mat_cpy(&view->projection, camera_get_projection_matrix(camera->camera));
		mat_cpy(&view->view, camera_get_view_matrix(camera->camera));

		// Calculate view-projection matrix for the camera.
		mat_multiply(
			view->projection,
			view->view,
			&view->view_projection
		);

		// Calculate inverse matrices.
		view->view_inv = mat_invert(view->view);
		view->projection_inv = mat_invert(view->projection);
		view->view_projection_inv = mat_invert(view->view_projection);

		view->view_position = vec3_to_vec4(obj_get_position(camera));
		view->ambient_light = col_to_vec4(scene->ambient_light);
		view->near = camera->camera->clip_near;
		view->far = camera->camera->clip_far;

		// Initialize a virtual root object.
		view->root.matrix = mat_identity();
		view->root.mvp = view->view_projection;

		// Apply post processing effects.
		shader_t *effect;
		
		arr_foreach(camera->camera->post_processing_effects, effect) {
			arr_push(view->post_processing_effects, effect);
		}

		// When rendering in deferred mode, add a list of lights affecting this view.
		// TODO: Actually check which lights are in the view! For now we're just copying all lights.
		if (is_using_deferred_lighting) {

			view->lights = mem_alloc_fast(lights.count * sizeof(rlight_t*));
			view->num_lights = lights.count;

			int light_index;

			arr_foreach_iter(lights, light_index) {
				view->lights[light_index] = lights.items[light_index];
			}
		}
		else {
			view->lights = NULL;
			view->num_lights = 0;
		}

		// Add the view to the list of views to be rendered.
		list_push(views, view);
	}

	// TODO: Find which scene objects are visible in the current camera (for now add all objects).
	// TODO: Sort transparent objects by depth! Otherwise if we render a transparent object first
	// which is in front of other objects, it will render the clear colour! (see fader sprite in the
	// example project)
	object_t *object;

	arr_foreach(scene->objects, object) {

		if (object != NULL && object->is_active) {
			rsys_cull_object(object);
		}
	}
}

void rsys_render_mesh(mesh_t *mesh, bool is_ui_mesh)
{
	if (mesh == NULL) {
		return;
	}

	rview_t *view;

	// Upload changed vertex data to the GPU.
	if (mesh->is_vertex_data_dirty &&
		mesh->handle_vertices != 0) {

		// Optimize sequential debug line data (which is usually updated every frame) by only
		// updating as many vertices as are needed to ne drawn.
		buffer_handle_t handle = mesh->handle_vertices;

		if (BUFFER_GET_INDEX(handle) == BUFIDX_DEBUG_LINE) {
			BUFFER_SET_SIZE(handle, mesh->vertex_size * mesh->num_indices_to_render);
		}

		bufcache_update(handle, mesh->vertices);
		mesh->is_vertex_data_dirty = false;
	}

	// TODO: Cull the mesh before adding it to the views!

	if (is_ui_mesh) {

		// Create a render mesh and mark the virtual UI root object as its parent.
		rmesh_t *rmesh = rsys_create_render_mesh(mesh, &ui_parent);

		// Here we just move the widget indices to the GPU. This is only done to UI
		// widgets as their indices are recalculated each frame.
		if (mesh->vertex_type == VERTEX_UI) {

			size_t num_indices = (mesh->num_indices_to_render ?
			                      mesh->num_indices_to_render : mesh->num_indices);
			
			rmesh->handle_indices = bufcache_alloc_indices(BUFIDX_UI, mesh->indices, num_indices);
		}

		// Add the mesh to a render queue determined by its shader.
		list_push(ui_view->meshes[rmesh->shader->queue], rmesh);

	}
	else {

		list_foreach(views, view) {

			// Create a separate render mesh for each view.
			rmesh_t *rmesh = rsys_create_render_mesh(mesh, &view->root);
			
			// Add the mesh to a render queue determined by its shader.
			list_push(view->meshes[rmesh->shader->queue], rmesh);
		}
	}
}

static void rsys_cull_object(object_t *object)
{
	if (object == NULL) {
		return;
	}

	// Skip non-visible objects.
	if (object->model != NULL ||
		object->sprite != NULL ||
		object->emitter != NULL) {

		// Add the scene object to each of the views as a render object.
		rview_t *view;

		list_foreach(views, view) {

			NEW(robject_t, obj);

			// Copy matrices.
			mat_cpy(&obj->matrix, obj_get_transform(object));

			mat_multiply(
				view->view_projection,
				obj->matrix,
				&obj->mvp);

			list_push(view->objects, obj);

			// Cull object meshes.
			rsys_cull_object_meshes(object, obj, view);
		}
	}
	
	// Add all of the child objects, too.
	object_t *child;

	arr_foreach(object->children, child) {
		rsys_cull_object(child);
	}
}

static void rsys_cull_object_meshes(object_t *object, robject_t *parent, rview_t *view)
{
	// TODO: HANDLE ACTUAL CULLING HERE
	
	// 3D model meshes
	if (object->model != NULL) {

		mesh_t *mesh;
		arr_foreach(object->model->meshes, mesh) {

			if (mesh != NULL) {
				rsys_add_mesh_to_view(mesh, object, parent, view);
			}
		}
	}

	// 2D sprite mesh
	if (object->sprite != NULL && object->sprite->mesh != NULL) {
		rsys_add_mesh_to_view(object->sprite->mesh, object, parent, view);
	}

	// Particle emitter mesh(es)
	if (object->emitter != NULL) {

		// That includes subemitters.
		for (uint32_t i = object->emitter->subemitters.count; i > 0; i--) {

			emitter_t *subemitter = object->emitter->subemitters.items[i - 1].emitter;

			if (subemitter != NULL &&
				subemitter->is_active &&
				subemitter->mesh != NULL &&
				subemitter->mesh->num_indices_to_render != 0) {

				rsys_add_mesh_to_view(subemitter->mesh, object, parent, view);
			}
		}

		// Main mesh.
		if (object->emitter->mesh != NULL &&
			object->emitter->is_active) {

			rsys_add_mesh_to_view(object->emitter->mesh, object, parent, view);
		}
	}
}

static void rsys_add_mesh_to_view(mesh_t *mesh, object_t *object, robject_t *parent, rview_t *view)
{
	// Upload vertex and data to the GPU. If the data is already copied to buffer objects,
	// refresh them to avoid automatic cleanup.

	// Vertex data
	if (mesh->vertex_buffer == NULL) {

		mesh->vertex_buffer = bufcache_legacy_alloc_buffer(
			mesh->vertices, mesh->num_vertices, mesh->vertex_size, false, true);

		mesh->is_vertex_data_dirty = false;
	}
	else {
		if (mesh->is_vertex_data_dirty) {

			// Reupload vertex data to the GPU.
			bufcache_legacy_upload_buffer(mesh->vertex_buffer, mesh->vertices, mesh->num_vertices,
                                          mesh->vertex_size, false, false);

			mesh->is_vertex_data_dirty = false;
		}
	}

	// Index data.
	if (mesh->index_buffer == NULL) { 

		mesh->index_buffer = bufcache_legacy_alloc_buffer(
			mesh->indices, mesh->num_indices, sizeof(vindex_t), true, true);
	}
	else {
		if (mesh->is_index_data_dirty) {

			size_t num_indices = mesh->num_indices;

			if (mesh->num_indices_to_render != 0) {
				num_indices = mesh->num_indices_to_render;
			}

			bufcache_legacy_upload_buffer(mesh->index_buffer, mesh->indices, num_indices,
                                          sizeof(vindex_t), true, false);

			mesh->is_index_data_dirty = false;
		}
	}

	// Create a new render mesh as a copy for the renderer.
	NEW(rmesh_t, rmesh);

	rmesh->parent = parent;
	rmesh->vertex_type = mesh->vertex_type;
	rmesh->vertices = mesh->vertex_buffer;
	rmesh->indices = mesh->index_buffer;

	// Set shader and texture from material.
	if (mesh->material != NULL) {

		rmesh->shader = mesh->material->shader;
		rmesh->texture = mesh->material->texture;
		rmesh->normal_map = mesh->material->normal_map;
	}

	// If the mesh defines override texture or shader, use them.
	rmesh->shader = (mesh->shader != NULL ? mesh->shader : rmesh->shader);
	rmesh->texture = (mesh->texture != NULL ? mesh->texture : rmesh->texture);

	// Use default shader which renders the mesh in purple if no other shader is defined.
	if (rmesh->shader == NULL) {
		rmesh->shader = default_shader;
	}

	// If the material used by the mesh is affected by lighting, collect all the lights contributing
	// to the mesh. This is only done if the rendering mode is forward - all lights are rendered
	// as a post process effect in the deferred lighting mode.
	if (shader_is_affected_by_light(rmesh->shader) &&
		!is_using_deferred_lighting) {

		rsys_collect_mesh_lights(rmesh, object);
	}

	// Add the mesh to the view to a render queue determined by its shader.
	list_push(view->meshes[rmesh->shader->queue], rmesh);
}

static rmesh_t *rsys_create_render_mesh(mesh_t *mesh, robject_t *root)
{
	// Create a new render mesh as a copy for the renderer.
	NEW(rmesh_t, rmesh);

	rmesh->parent = root;
	rmesh->vertex_type = mesh->vertex_type;
	rmesh->handle_vertices = mesh->handle_vertices;
	rmesh->handle_indices = mesh->handle_indices;

	// Override the number of indices to render.
	if (mesh->num_indices_to_render != 0) {

		BUFFER_SET_SIZE(
			rmesh->handle_indices,
			sizeof(vindex_t) * (uint64_t)mesh->num_indices_to_render
		);
	}

	// Set shader and texture from material.
	if (mesh->material != NULL) {

		rmesh->shader = mesh->material->shader;
		rmesh->texture = mesh->material->texture;
		rmesh->normal_map = mesh->material->normal_map;
	}

	// If the mesh defines override texture or shader, use them.
	rmesh->shader = (mesh->shader != NULL ? mesh->shader : rmesh->shader);
	rmesh->texture = (mesh->texture != NULL ? mesh->texture : rmesh->texture);

	// Use default shader which renders the mesh in purple if no other shader is defined.
	if (rmesh->shader == NULL) {
		rmesh->shader = default_shader;
	}

	return rmesh;
}

static void rsys_collect_mesh_lights(rmesh_t *render_mesh, object_t *parent)
{
	// TODO: This part currently adds all the nearest lights to the mesh. Once the meshes and
	// objects have bounding boxes, see whether the light even reaches the mesh at all.
	rlight_t *contributing_lights[100];
	uint32_t num_lights = 0;

	vec3_t position = obj_get_position(parent);
	rlight_t *light;

	arr_foreach(lights, light) {

		light_t *scene_light = light->light;

		if (scene_light->type == LIGHT_DIRECTIONAL) {

			// Directional lights will always affect the mesh.
			light->dist_sq = 0;
		}
		else {

			// Calculate the (squared) distance of the light to the mesh.
			// TODO: Take into account the cone of a spotlight!
			vec3_t light_position = obj_get_position(scene_light->parent);
			light->dist_sq = vec3_distance_sq(position, light_position);
		}
		
		// If the mesh is within the light's range, add the light to the list of potential
		// light sources.
		if (light->dist_sq < SQUARED(scene_light->range)) {

			contributing_lights[num_lights++] = light;

			if (num_lights >= 100) {
				break;
			}
		}
	}

	// No lights contributing to the lighting of the mesh.
	if (num_lights == 0) {

		render_mesh->num_lights = 0;
		return;
	}

	// If there are more lights contributing to the lighting of the mesh than our shaders would
	// support, select only the closest ones.
	// TODO: Select only the highest intensity ones?
	if (num_lights > MAX_LIGHTS_PER_MESH) {

		qsort(contributing_lights,
		      num_lights,
		      sizeof(contributing_lights[0]),
		      rsys_sort_lights_by_distance
		);
	}

	uint32_t count = MIN(num_lights, MAX_LIGHTS_PER_MESH);

	render_mesh->num_lights = count;

	// Copy references to the contributing render lights to the mesh.
	for (uint32_t i = 0; i < count; i++) {
		render_mesh->lights[i] = contributing_lights[i];
	}
}

static int rsys_sort_lights_by_distance(const void *p1, const void *p2)
{
	const rlight_t *light1 = *(const rlight_t **)p1;
	const rlight_t *light2 = *(const rlight_t **)p2;

	if (light1->dist_sq < light2->dist_sq) return -1;
	if (light1->dist_sq > light2->dist_sq) return 1;
	return 0;
}

static void rsys_free_frame_data(void)
{
	// Remove all regular render views.
	rview_t *view, *tmp_view;

	list_foreach_safe(views, view, tmp_view) {

		// Remove all mesh copies in the view.
		rmesh_t *mesh, *tmp;

		for (int i = 0; i < NUM_QUEUES; i++) {

			list_foreach_safe(view->meshes[i], mesh, tmp) {
				mem_free(mesh);
			}
		}

		// Remove all object copies in the view.
		robject_t *obj, *tmp2;

		list_foreach_safe(view->objects, obj, tmp2) {
			mem_free(obj);
		}

		arr_clear(view->post_processing_effects);

		mem_free(view->lights);

		// Remove the view itself.
		mem_free(view);
	}

	list_clear(views);
	ui_view = NULL;

	// Clear the UI index buffer for rebuilding during the next frame.
	bufcache_clear_all_indices(BUFIDX_UI);

	// Clear light data.
	rlight_t *light;

	arr_foreach(lights, light) {
		DESTROY(light);
	}

	arr_clear(lights);
}
