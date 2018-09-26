#include "extensions.h"
#include "io/log.h"
#include <string.h>

typedef void (*extension_t)(void);

// --------------------------------------------------------------------------------
// Pointers to OpenGL extension methods

// ARB_vertex_buffer_object functions
PFNGLGENBUFFERSARBPROC glGenBuffersARB;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
PFNGLBINDBUFFERARBPROC glBindBufferARB;
PFNGLBUFFERDATAARBPROC glBufferDataARB;
PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;

// ARB_shader_objects functions
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocation;
PFNGLUSEPROGRAMPROC glUseProgram;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORM1IARBPROC glUniform1i;
PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArray;

// --------------------------------------------------------------------------------

static bool glext_is_supported(const char *extension);
static extension_t glext_get_method(const char *name);

// --------------------------------------------------------------------------------

bool glext_initialize(void)
{
	// Make sure the underlying platform and hardware support all the features required by the renderer.
	bool ARB_vertex_buffer_object = glext_is_supported("GL_ARB_vertex_buffer_object");
	bool ARB_shader_objects = glext_is_supported("GL_ARB_shader_objects");
	bool ARB_vertex_shader = glext_is_supported("GL_ARB_vertex_shader");
	bool ARB_fragment_shader = glext_is_supported("GL_ARB_fragment_shader");

	if (!ARB_vertex_buffer_object ||
		!ARB_shader_objects ||
		!ARB_vertex_shader ||
		!ARB_fragment_shader) {

		log_error("OpenGL", "Missing essential extensions.");
		return false;
	}

	// GL_ARB_vertex_buffer_object
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)glext_get_method("glGenBuffersARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)glext_get_method("glDeleteBuffersARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC)glext_get_method("glBindBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC)glext_get_method("glBufferDataARB");
	glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)glext_get_method("glBufferSubDataARB");

	// GL_ARB_shader_objects
	glCreateShader = (PFNGLCREATESHADERPROC)glext_get_method("glCreateShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)glext_get_method("glDeleteShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)glext_get_method("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)glext_get_method("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)glext_get_method("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glext_get_method("glGetShaderInfoLog");

	glCreateProgram = (PFNGLCREATEPROGRAMPROC)glext_get_method("glCreateProgram");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glext_get_method("glDeleteProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)glext_get_method("glLinkProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)glext_get_method("glAttachShader");
	glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC)glext_get_method("glBindFragDataLocation");
	glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONARBPROC)glext_get_method("glBindAttribLocation");
	glUseProgram = (PFNGLUSEPROGRAMPROC)glext_get_method("glUseProgram");

	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glext_get_method("glGetUniformLocation");
	glUniform1f = (PFNGLUNIFORM1FPROC)glext_get_method("glUniform1f");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)glext_get_method("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)glext_get_method("glUniform4fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)glext_get_method("glUniformMatrix4fv");
	glUniform1i = (PFNGLUNIFORM1IARBPROC)glext_get_method("glUniform1i");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERARBPROC)glext_get_method("glVertexAttribPointer");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)glext_get_method("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)glext_get_method("glDisableVertexAttribArray");

	return true;
}

static bool glext_is_supported(const char *name)
{
	const char *extensions = (const char *)glGetString(GL_EXTENSIONS);

	if (strstr(extensions, name) == NULL) {
		return false;
	}

	return true;
}

#ifdef _WIN32
#error "Missing implementation for OpenGL extensions"
#else

static extension_t glext_get_method(const char *name)
{
	extension_t proc = glXGetProcAddress((const GLubyte *)name);

	if (proc == NULL) {
		log_note("OpenGL", "Unable to load extension method '%s'", name);
	}

	return proc;
}

#endif
