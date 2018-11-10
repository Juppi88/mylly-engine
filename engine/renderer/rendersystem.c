#include "rendersystem.h"
#include "renderer.h"
#include "vbcache.h"
#include "renderview.h"
#include "model.h"
#include "shader.h"
#include "texture.h"
#include "vertexbuffer.h"
#include "scene/scene.h"
#include "scene/object.h"
#include "scene/camera.h"
#include "io/log.h"

// --------------------------------------------------------------------------------

static int frames_rendered; // Number of frames rendered so far
static stack_t(rview_t) views; // List of views to be rendered this frame

// --------------------------------------------------------------------------------

static void rsys_cull_object(object_t *object);
static void rsys_cull_meshes(rview_t *view);
static void rsys_free_frame_data(void);

// --------------------------------------------------------------------------------

void rsys_initialize(void)
{
	// Initialize renderer backend.
	rend_initialize();
	vbcache_initialize();

	log_message("RenderSystem", "Rendering system initialized.");
}

void rsys_shutdown(void)
{
	vbcache_shutdown();
	rend_shutdown();
}

void rsys_begin_frame(void)
{
	vbcache_set_current_frame(frames_rendered);
}

void rsys_end_frame(void)
{
	// Issue the actual draw calls here.
	rend_draw_views(views);

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
		stack_push(views, view);
	}

	// TODO: Find which scene objects are visible in the current camera (for now add all objects).
	object_t *object;

	arr_foreach(scene->objects, object) {
		rsys_cull_object(object);
	}

	// Cull all meshes which aren't in the view.
	stack_foreach(rview_t, view, views) {
		rsys_cull_meshes(views);
	}
}

static void rsys_cull_object(object_t *object)
{
	if (object == NULL) {
		return;
	}

	// Skip non-visible objects.
	if (object->model != NULL) {

		// Add the scene object to each of the views as a render object.
		stack_foreach(rview_t, view, views) {

			NEW(robject_t, obj);

			// Copy model data.
			obj->model = object->model;

			// Copy matrices.
			mat_cpy(&obj->matrix, obj_get_transform(object));

			mat_multiply(
				view->projection,
				obj->matrix,
				&obj->mvp);

			stack_push(view->objects, obj);
		}
	}
	
	// Add all of the child objects, too.
	object_t *child;

	arr_foreach(object->children, child) {
		rsys_cull_object(child);
	}
}

static void rsys_cull_meshes(rview_t *view)
{
	shader_t *default_shader = res_get_shader("default");

	// TODO: HANDLE ACTUAL CULLING HERE
	stack_foreach(robject_t, obj, view->objects) {

		mesh_t *mesh;

		arr_foreach(obj->model->meshes, mesh) {

			// Upload vertex and data to the GPU. If the data is already copied to buffer objects,
			// refresh them to avoid automatic cleanup.

			// Vertex data
			if (mesh->vertex_buffer == NULL) {

				vbcache_alloc_buffer(mesh->vertices, mesh->num_vertices,
									sizeof(vertex_t), &mesh->vertex_buffer, false);
			}
			else {
				vbcache_refresh_buffer(mesh->vertex_buffer);
			}

			// Index data.
			if (mesh->index_buffer == NULL) { 

				vbcache_alloc_buffer(mesh->indices, mesh->num_indices,
									sizeof(vertex_t), &mesh->index_buffer, true);
			}
			else {
				vbcache_refresh_buffer(mesh->index_buffer);
			}

			// Create a new render mesh as a copy for the renderer.
			NEW(rmesh_t, rmesh);

			rmesh->parent = obj;
			rmesh->vertices = mesh->vertex_buffer;
			rmesh->indices = mesh->index_buffer;

			// Use default shader until others are available.
			rmesh->shader = (mesh->shader != NULL ? mesh->shader : default_shader);
			rmesh->texture = mesh->texture;

			// Add the mesh to the view.
			stack_push(view->meshes, rmesh);
		}
	}
}

static void rsys_free_frame_data(void)
{
	for (rview_t *view = stack_first(views), *tmp = NULL; view != NULL; view = tmp) {

		tmp = stack_next(view);

		// Remove all mesh copies in the view.
		stack_foreach_safe(rmesh_t, mesh, view->meshes) {

			stack_foreach_safe_begin(mesh);
			mem_free(mesh);
		}

		// Remove all object copies in the view.
		stack_foreach_safe(robject_t, obj, view->objects) {

			stack_foreach_safe_begin(obj);
			mem_free(obj);
		}

		// Remove the view itself.
		mem_free(view);
	}

	views = NULL;
}
