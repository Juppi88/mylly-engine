#ifndef __OPENGL_DEFAULT_SHADERS_INL
#define __OPENGL_DEFAULT_SHADERS_INL

// -------------------------------------------------------------------------------------------------

// The source code for a default GLSL shader which renders everything in purple.
// Used when no valid shaders are available.
const char *default_shader_source =

"uniform mat4 MatrixArr[1];\n"
"\n"
"#if defined(VERTEX_SHADER)\n"
"\n"
"attribute vec3 Vertex;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = MatrixArr[0] * vec4(Vertex, 1.0);\n"
"}\n"
"\n"
"#elif defined(FRAGMENT_SHADER)\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = vec4(1, 0, 1, 1);\n"
"}\n"
"\n"
"#endif\n";

// -------------------------------------------------------------------------------------------------

// The source code for a minimal GLSL shader rendering a textured quad with alpha blend to fixed
// background colour. Used for drawing the splash screen.
const char *splash_shader_source =

"uniform sampler2D Texture;\n"
"uniform vec4 Colour;\n"
"varying vec2 texCoord;\n"
"\n"
"#if defined(VERTEX_SHADER)\n"
"\n"
"attribute vec2 Vertex;\n"
"attribute vec2 TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(Vertex, 0.0, 1.0);\n"
"	texCoord = TexCoord;\n"
"}\n"
"\n"
"#elif defined(FRAGMENT_SHADER)\n"
"\n"
"void main()\n"
"{\n"
"	vec4 texColour = texture2D(Texture, texCoord);\n"
"\n"
"	if (texColour.a < 0.01) { discard; }\n"
"\n"
"	vec3 outColour = (1.0 - texColour.a) * Colour.rgb + texColour.a * texColour.rgb;\n"
"	outColour = (1.0 - Colour.a) * Colour.rgb + Colour.a * outColour.rgb;\n"
"	gl_FragColor = vec4(outColour, 1);\n"
"}\n"
"\n"
"#endif\n";

// -------------------------------------------------------------------------------------------------

// The source of a minimal GLSL shader for drawing the contents of a framebuffer to the screen.
const char *draw_fb_shader_source =

"uniform sampler2D SamplerArr[1];\n"
"varying vec2 texCoord;\n"
"\n"
"#if defined(VERTEX_SHADER)\n"
"\n"
"attribute vec2 Vertex;\n"
"attribute vec2 TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(Vertex, 0.0, 1.0);\n"
"	texCoord = TexCoord;\n"
"}\n"
"\n"
"#elif defined(FRAGMENT_SHADER)\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = vec4(texture2D(SamplerArr[0], texCoord).rgb, 1);\n"
"}\n"
"\n"
"#endif\n";

// -------------------------------------------------------------------------------------------------

// Same as above but only draws the alpha channel.
const char *draw_fb_alpha_shader_source =

"uniform sampler2D SamplerArr[1];\n"
"varying vec2 texCoord;\n"
"\n"
"#if defined(VERTEX_SHADER)\n"
"\n"
"attribute vec2 Vertex;\n"
"attribute vec2 TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(Vertex, 0.0, 1.0);\n"
"	texCoord = TexCoord;\n"
"}\n"
"\n"
"#elif defined(FRAGMENT_SHADER)\n"
"\n"
"void main()\n"
"{\n"
"	float a = texture2D(SamplerArr[0], texCoord).a;\n"
"	gl_FragColor = vec4(a, a, a, 1);\n"
"}\n"
"\n"
"#endif\n";

#endif
