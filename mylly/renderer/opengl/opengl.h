#pragma once
#ifndef __MYLLY_OPENGL_H
#define __MYLLY_OPENGL_H

#include "core/defines.h"

// Include OpenGL headers
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

#endif
