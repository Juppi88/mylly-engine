#pragma queue TRANSPARENT

varying vec4 colour;

#if defined(VERTEX_SHADER)

// Attributes for UI widget quads.
attribute vec3 Vertex;
attribute vec4 Colour;

uniform mat4 MatrixArr[1];

void main()
{
	gl_Position = MatrixArr[0] * vec4(Vertex, 1);
	gl_Position.z = -1.0; // Render the line on top of everything.

	colour = Colour;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	gl_FragColor = colour;
}

#endif
