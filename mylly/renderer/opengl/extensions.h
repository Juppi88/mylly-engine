#pragma once
#ifndef __OPENGL_EXTENSIONS_H
#define __OPENGL_EXTENSIONS_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

#ifdef _WIN32
	#include <Windows.h>
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
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
extern PFNGLUNIFORM1IARBPROC glUniform1i;
extern PFNGLUNIFORM1IVPROC glUniform1iv;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArray;

// Render buffers
extern PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
extern PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffers;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorage;

// Frame buffers
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;

// Misc functions
extern PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;

extern PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElementsARB;

#ifdef _WIN32
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif

// --------------------------------------------------------------------------------

bool glext_initialize(void);

END_DECLARATIONS;

#endif
