#pragma once
#ifndef __OPENGL_EXTENSIONS_H
#define __OPENGL_EXTENSIONS_H

#include "core/defines.h"

#ifdef _WIN32
	#include <Windows.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glext.h>
#else
	#include <GL/gl.h>
	#include <GL/glx.h>
	#include <GL/glu.h>
#endif

// --------------------------------------------------------------------------------

//
// Extensions
//

// ARB_vertex_buffer_object
extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;

// ARB_shader_objects
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
extern PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocation;
extern PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocation;
extern PFNGLUSEPROGRAMPROC glUseProgram;

extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLUNIFORM1IARBPROC glUniform1i;
extern PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArray;

// --------------------------------------------------------------------------------

bool glext_initialize(void);

#endif
