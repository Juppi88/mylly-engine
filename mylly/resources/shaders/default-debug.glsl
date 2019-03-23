#pragma queue TRANSPARENT

uniform mat4 MatrixMVP;
varying vec4 colour;

#if defined(VERTEX_SHADER)

// Attributes for UI widget quads.
attribute vec3 Vertex;
attribute vec4 Colour;

void main()
{
	gl_Position = MatrixMVP * vec4(Vertex, 1);
	gl_Position.z = -1.0; // Render the line on top of everything.

	colour = Colour;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	gl_FragColor = colour;
}

#endif
