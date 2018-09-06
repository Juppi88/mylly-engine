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

void rend_draw_view(const model_t *model)
{
	rend_begin_draw();

	glEnableClientState(GL_VERTEX_ARRAY);

	array_foreach(mesh_t*, model->meshes, mesh) {

		glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), mesh->vertices);
		glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_SHORT, mesh->indices);
	}
	array_end();

	/*LIST_FOREACH(vertexbuffer_t, vb, buffers)
	{
		for (int i = 0; i < vb->count; i++) {

			vertex_t *v = &vb->buffer[i];
			glColor3f(v->r / 255.0f, v->g / 255.0f, v->b / 255.0f);
			glVertex3f(v->pos.x, v->pos.y, v->pos.z);
		}
	}*/

	glDisableClientState(GL_VERTEX_ARRAY);

	rend_end_draw();
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
