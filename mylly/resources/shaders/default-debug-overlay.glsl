#pragma queue OVERLAY

varying vec4 colour;

#if defined(VERTEX_SHADER)

// Attributes for debug primitives.
attribute vec3 Vertex;
attribute vec4 Colour;

// To avoid having to include the entire engine include file, we'll define the matrix array here.
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
	gl_FragData[0] = colour;
}

#endif
