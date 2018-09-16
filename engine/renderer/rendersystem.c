#include "rendersystem.h"
#include "renderer.h"
#include "vbcache.h"
#include "renderview.h"
#include "io/log.h"

// --------------------------------------------------------------------------------

static int frames_rendered; // Number of frames rendered so far
static LIST(rview_t) views; // List of views to be rendered this frame

static shader_t *default_shader; // Shader used by default

// --------------------------------------------------------------------------------

static void rsys_process_mesh_frustrum_culling(rview_t *view);
static void rsys_free_frame_data(void);

// --------------------------------------------------------------------------------

void rsys_initialize(void)
{
	// Initialize renderer backend.
	rend_initialize();
	vbcache_initialize();

	// Create a default shader to be used when nothing else is available.
	default_shader = shader_create("default", rend_get_default_shader_source());

	log_message("RenderSystem", "Rendering system initialized.");
}

void rsys_shutdown(void)
{
	shader_destroy(default_shader);
	default_shader = NULL;

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

void rsys_render_scene(model_t *model)
{
	// Collect info about the objects in the scene before rendering anything and process culling etc.
	// TODO: Also use a proper temp allocator because this is alloc heavy!

	// TODO: Add a single view for each camera!
	NEW(rview_t, view);

	// TODO: Find which scene objects are visible in the current camera.
	// Test code: We don't have a scene structure yet, so we're just rendering the test object.
	{
		NEW(robject_t, obj);
		obj->model = model;

		LIST_ADD(view->objects, obj);
	}

	// Cull all meshes which aren't in the view.
	rsys_process_mesh_frustrum_culling(view);

	// Add the view to the list of views to be rendered.
	LIST_ADD(views, view);
}

static void rsys_process_mesh_frustrum_culling(rview_t *view)
{
	// TODO: ACTUAL CULLING HERE
	LIST_FOREACH(robject_t, obj, view->objects) {

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
			rmesh->shader = default_shader; // Use default shader until others are available.

			// Add the mesh to the view.
			LIST_ADD(view->meshes, rmesh);
		}
	}
}

static void rsys_free_frame_data(void)
{
	for (rview_t *view = LIST_FIRST(views), *tmp = NULL; view != NULL; view = tmp) {

		tmp = LIST_NEXT(view);

		// Remove all mesh copies in the view.
		LIST_FOREACH_SAFE(rmesh_t, mesh, view->meshes) {

			LIST_FOREACH_SAFE_BEGIN(mesh);
			mem_free(mesh);
		}

		// Remove all object copies in the view.
		LIST_FOREACH_SAFE(robject_t, obj, view->objects) {

			LIST_FOREACH_SAFE_BEGIN(obj);
			mem_free(obj);
		}

		// Remove the view itself.
		mem_free(view);
	}
}
