#include "renderer/renderer.h"
#include "extensions.h"
#include "renderer/vertex.h"
#include "io/log.h"
#include "platform/window.h"

#ifdef _WIN32
#error "Missing implementation"
#else
static GLXContext context;
#endif

// --------------------------------------------------------------------------------

static void rend_begin_draw(void);
static void rend_end_draw(void);

// --------------------------------------------------------------------------------

bool rend_initialize(void)
{
	// Create an OpenGL rendering context.	
#ifdef _WIN32
#error "Missing implementation"
#else
	context = glXCreateContext(window_get_display(), window_get_visual_info(), NULL, GL_TRUE);
 
	if (context == NULL) {
		log_error("Renderer", "Could not create an OpenGL rendering context");
		return false;
	}

	glXMakeCurrent(window_get_display(), window_get_handle(), context);
#endif

	if (!glext_initialize()) {

		log_error("Renderer", "Could not load all essential OpenGL extensions");
		return false;	
	}

	glEnable(GL_DEPTH_TEST);

	return true;
}

void rend_shutdown(void)
{
	// Destroy the rendering context.
#ifdef _WIN32
#error "Missing implementation"
#else
	glXMakeCurrent(window_get_display(), None, NULL);
	glXDestroyContext(window_get_display(), context);

	context = NULL;
#endif
}

void rend_draw_views(LIST(rview_t) views)
{
	rend_begin_draw();

	glEnableClientState(GL_VERTEX_ARRAY);

	LIST_FOREACH(rview_t, view, views) {

		LIST_FOREACH(rmesh_t, mesh, view->meshes) {

			/// Bind the meshes vertex buffer and set vertex data pointers.
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vertices->vbo);

			// Set vertex data offsets in the buffer.
			glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, pos));
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex_t), (void *)offsetof(vertex_t, colour));
			glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, uv));
			glNormalPointer(GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, normal));

			// Draw the triangles of the mesh.
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->indices->vbo);
			glDrawElements(GL_TRIANGLES, mesh->indices->count, GL_UNSIGNED_SHORT, 0);
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	rend_end_draw();
}

vbindex_t rend_generate_buffer(void)
{
	GLuint vbo;

	glGenBuffersARB(1, &vbo);
	return vbo;
}

void rend_destroy_buffer(vbindex_t vbo)
{
	if (vbo != 0) {
		glDeleteBuffersARB(1, &vbo);
	}
}

void rend_upload_buffer_data(vbindex_t vbo, void *data, size_t size, bool is_index_data)
{
	if (is_index_data) {
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, size, data, GL_STATIC_DRAW_ARB);
	}
	else {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, data, GL_STATIC_DRAW_ARB);
	}
}

static void rend_begin_draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 1.0f);
	glEnable(GL_BLEND);

	// Setup the camera.
	// TODO: Get this from frame data and move it to drawframe!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	gluOrtho2D(-2, 2, -2, 2);
}

static void rend_end_draw(void)
{
#ifdef _WIN32
#error "Missing implementation"
#else
	glXSwapBuffers(window_get_display(), window_get_handle());
#endif
}
