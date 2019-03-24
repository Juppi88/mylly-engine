#include "rendersystem.h"
#include "renderer.h"
#include "vbcache.h"
#include "renderview.h"
#include "shader.h"
#include "shaderdata.h"
#include "texture.h"
#include "vertexbuffer.h"
#include "buffercache.h"
#include "debug.h"
#include "scene/scene.h"
#include "scene/object.h"
#include "scene/model.h"
#include "scene/sprite.h"
#include "scene/emitter.h"
#include "scene/camera.h"
#include "resources/resources.h"
#include "core/mylly.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

static int frames_rendered; // Number of frames rendered so far
static shader_t *default_shader; // Default shader used for rendering when a mesh has no shader

static list_t(rview_t) views; // List of views to be rendered this frame

static rview_t *ui_view; // A dedicated view for UI widgets due to UI not needing a camera
static robject_t ui_parent; // A virtual object to be used as the UI parent

// -------------------------------------------------------------------------------------------------

static void rsys_cull_object(object_t *object);
static void rsys_cull_object_meshes(object_t *object, robject_t *parent, rview_t *view);
static void rsys_add_mesh_to_view(mesh_t *mesh, robject_t *parent, rview_t *view);
static rmesh_t *rsys_create_render_mesh(mesh_t *mesh, robject_t *root);
static void rsys_free_frame_data(void);

// -------------------------------------------------------------------------------------------------

void rsys_initialize(void)
{
	// Initialize renderer backend.
	rend_initialize();
	vbcache_initialize();
	bufcache_initialize();
	shader_data_initialize();

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

	log_message("RenderSystem", "Rendering system initialized.");
}

void rsys_shutdown(void)
{
	shader_data_shutdown();
	bufcache_shutdown();
	vbcache_shutdown();
	rend_shutdown();
}

void rsys_begin_frame(void)
{
	// Attempt to load the default shader if it has not been loaded.
	if (default_shader == NULL) {
		default_shader = res_get_shader("default");
	}

	vbcache_set_current_frame(frames_rendered);

	rend_begin_draw();
	debug_begin_frame();

	// Setup a per-frame view object for the UI.
	NEW(rview_t, view);
	ui_view = view;

	mat_cpy(&ui_view->projection, &ui_parent.mvp);
}

void rsys_end_frame(void)
{
	// Generate and issue the actual draw calls here. All scenes and their objects ahave already
	// been added to the views, soo all that's left are special meshes (debug primitives,
	// UI widgets).

	// First collect debug primitives.
	debug_end_frame();

	// Add the UI view to the view list as last.
	list_push(views, ui_view);

	// Finalize by issuing the render views to the renderer backend.
	rend_draw_views(views.first);
	rend_end_draw();

	// Release all temporary data.
	rsys_free_frame_data();

	// Free all inactive vertex buffer objects for the next frame.
	vbcache_free_inactive_buffers();

	++frames_rendered;
}

void rsys_render_scene(scene_t *scene)
{
	if (scene == NULL) {
		return;
	}

	// Collect info about the objects in the scene before rendering anything and process culling etc.
	// TODO: Also use a proper temp allocator because this is alloc heavy!

	// Create a separate render view for every camera in the scene.
	object_t *camera;

	arr_foreach(scene->cameras, camera) {

		if (camera == NULL) {
			continue;
		}

		NEW(rview_t, view);

		// Calculate view-projection matrix for the camera.
		mat_multiply(
			*camera_get_projection_matrix(camera->camera),
			*camera_get_view_matrix(camera->camera),
			&view->projection
		);

		// Initialize a virtual root object.
		view->root.matrix = mat_identity();
		view->root.mvp = view->projection;

		// Add the view to the list of views to be rendered.
		list_push(views, view);
	}

	// TODO: Find which scene objects are visible in the current camera (for now add all objects).
	object_t *object;

	arr_foreach(scene->objects, object) {
		rsys_cull_object(object);
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
				view->projection,
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
				rsys_add_mesh_to_view(mesh, parent, view);
			}
		}
	}

	// 2D sprite mesh
	if (object->sprite != NULL && object->sprite->mesh != NULL) {
		rsys_add_mesh_to_view(object->sprite->mesh, parent, view);
	}

	// Particle emitter mesh
	if (object->emitter != NULL && object->emitter->mesh != NULL) {
		rsys_add_mesh_to_view(object->emitter->mesh, parent, view);
	}
}

static void rsys_add_mesh_to_view(mesh_t *mesh, robject_t *parent, rview_t *view)
{
	// Upload vertex and data to the GPU. If the data is already copied to buffer objects,
	// refresh them to avoid automatic cleanup.

	// Vertex data
	if (mesh->vertex_buffer == NULL) {

		vbcache_alloc_buffer(mesh->vertices, mesh->num_vertices, mesh->vertex_size,
                             &mesh->vertex_buffer, false, true);

		mesh->is_vertex_data_dirty = false;
	}
	else {
		if (mesh->is_vertex_data_dirty) {

			// Reupload vertex data to the GPU.
			vbcache_upload_buffer(mesh->vertex_buffer, mesh->vertices, mesh->num_vertices,
                                  mesh->vertex_size, false, false);

			mesh->is_vertex_data_dirty = false;
		}

		vbcache_refresh_buffer(mesh->vertex_buffer);
	}

	// Index data.
	if (mesh->index_buffer == NULL) { 

		vbcache_alloc_buffer(mesh->indices, mesh->num_indices,
                             sizeof(vindex_t), &mesh->index_buffer, true, true);
	}
	else {
		vbcache_refresh_buffer(mesh->index_buffer);
	}

	// Create a new render mesh as a copy for the renderer.
	NEW(rmesh_t, rmesh);

	rmesh->parent = parent;
	rmesh->vertex_type = mesh->vertex_type;
	rmesh->vertices = mesh->vertex_buffer;
	rmesh->indices = mesh->index_buffer;

	// Use default shader until others are available.
	rmesh->shader = (mesh->shader != NULL ? mesh->shader : default_shader);
	rmesh->texture = mesh->texture;

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

	// Use default shader unless others are available.
	rmesh->shader = (mesh->shader != NULL ? mesh->shader : default_shader);
	rmesh->texture = mesh->texture;

	return rmesh;
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

		// Remove the view itself.
		mem_free(view);
	}

	list_clear(views);
	ui_view = NULL;

	// Clear the UI index buffer for rebuilding during the next frame.
	bufcache_clear_all_indices(BUFIDX_UI);
}
