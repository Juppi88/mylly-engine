#include "rendersystem.h"
#include "renderer.h"
#include "vbcache.h"
#include "renderview.h"
#include "shader.h"
#include "texture.h"
#include "vertexbuffer.h"
#include "scene/scene.h"
#include "scene/object.h"
#include "scene/model.h"
#include "scene/sprite.h"
#include "scene/emitter.h"
#include "scene/camera.h"
#include "resources/resources.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

static int frames_rendered; // Number of frames rendered so far
static list_t(rview_t) views; // List of views to be rendered this frame
static shader_t *default_shader; // Default shader used for rendering when a mesh has no shader

// -------------------------------------------------------------------------------------------------

static void rsys_cull_object(object_t *object);
static void rsys_cull_object_meshes(object_t *object, robject_t *parent, rview_t *view);
static void rsys_add_mesh_to_view(mesh_t *mesh, robject_t *parent, rview_t *view);
static void rsys_free_frame_data(void);

// -------------------------------------------------------------------------------------------------

void rsys_initialize(void)
{
	// Initialize renderer backend.
	rend_initialize();
	vbcache_initialize();
	bufcache_initialize();

	log_message("RenderSystem", "Rendering system initialized.");
}

void rsys_shutdown(void)
{
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
}

void rsys_end_frame(void)
{
	// Issue the actual draw calls here.
	rend_draw_views(views.first);

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

		// Add the view to the list of views to be rendered.
		list_push(views, view);
	}

	// TODO: Find which scene objects are visible in the current camera (for now add all objects).
	object_t *object;

	arr_foreach(scene->objects, object) {
		rsys_cull_object(object);
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

static void rsys_free_frame_data(void)
{
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
}
